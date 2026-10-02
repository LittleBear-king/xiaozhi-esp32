#include "watch_voice_stream.h"

#include <cJSON.h>
#include <esp_log.h>
#include <web_socket.h>

#include "board.h"

namespace {
constexpr char kTag[] = "WatchVoice";
}

WatchVoiceStream::WatchVoiceStream() = default;

WatchVoiceStream::~WatchVoiceStream() {
    Close();
}

bool WatchVoiceStream::Connect(const std::string& url) {
    Close();
    auto network = Board::GetInstance().GetNetwork();
    if (network == nullptr) {
        ESP_LOGE(kTag, "Network is unavailable");
        return false;
    }

    websocket_ = network->CreateWebSocket(1);
    if (websocket_ == nullptr) {
        ESP_LOGE(kTag, "Failed to create WebSocket");
        return false;
    }
    websocket_->OnData([this](const char* data, size_t length, bool binary) {
        if (binary) {
            if (audio_callback_ != nullptr) {
                audio_callback_(reinterpret_cast<const uint8_t*>(data), length);
            }
            return;
        }
        HandleText(data, length);
    });
    websocket_->OnDisconnected([this]() {
        started_ = false;
        if (event_callback_ != nullptr) {
            event_callback_("disconnected", "");
        }
    });

    auto result = websocket_->Connect(url.c_str());
    if (!result) {
        ESP_LOGE(kTag, "Connect failed: %s", result.error().ToString().c_str());
        websocket_.reset();
        return false;
    }
    return true;
}

bool WatchVoiceStream::Begin(const std::string& language, int sample_rate, int channels) {
    if (!IsConnected() || started_) {
        return false;
    }
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "start");
    cJSON_AddStringToObject(root, "language", language.c_str());
    cJSON_AddStringToObject(root, "format", "pcm_s16le");
    cJSON_AddNumberToObject(root, "sample_rate", sample_rate);
    cJSON_AddNumberToObject(root, "channels", channels);
    char* json = cJSON_PrintUnformatted(root);
    bool sent = websocket_->Send(std::string(json));
    cJSON_free(json);
    cJSON_Delete(root);
    started_ = sent;
    return sent;
}

bool WatchVoiceStream::SendPcm(const int16_t* samples, size_t sample_count) {
    if (!IsConnected() || !started_ || samples == nullptr || sample_count == 0) {
        return false;
    }
    return websocket_->Send(reinterpret_cast<const char*>(samples), sample_count * sizeof(int16_t), true);
}

bool WatchVoiceStream::End() {
    if (!IsConnected() || !started_) {
        return false;
    }
    started_ = false;
    return websocket_->Send("{\"type\":\"end\"}");
}

void WatchVoiceStream::Close() {
    started_ = false;
    websocket_.reset();
}

bool WatchVoiceStream::IsConnected() const {
    return websocket_ != nullptr && websocket_->IsConnected();
}

void WatchVoiceStream::HandleText(const char* data, size_t length) {
    cJSON* root = cJSON_ParseWithLength(data, length);
    if (root == nullptr) {
        ESP_LOGW(kTag, "Invalid server event");
        return;
    }
    const cJSON* type = cJSON_GetObjectItem(root, "type");
    const cJSON* text = cJSON_GetObjectItem(root, "text");
    if (event_callback_ != nullptr && cJSON_IsString(type)) {
        event_callback_(type->valuestring, cJSON_IsString(text) ? text->valuestring : "");
    }
    cJSON_Delete(root);
}
