#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

class WebSocket;

class WatchVoiceStream {
public:
    using EventCallback = std::function<void(const std::string& type, const std::string& text)>;
    using AudioCallback = std::function<void(const uint8_t* data, size_t size)>;

    WatchVoiceStream();
    ~WatchVoiceStream();

    WatchVoiceStream(const WatchVoiceStream&) = delete;
    WatchVoiceStream& operator=(const WatchVoiceStream&) = delete;

    bool Connect(const std::string& url);
    bool Begin(const std::string& language = "zh", int sample_rate = 16000, int channels = 1);
    bool SendPcm(const int16_t* samples, size_t sample_count);
    bool End();
    void Close();
    bool IsConnected() const;

    void SetEventCallback(EventCallback callback) { event_callback_ = std::move(callback); }
    void SetAudioCallback(AudioCallback callback) { audio_callback_ = std::move(callback); }

private:
    void HandleText(const char* data, size_t length);

    std::unique_ptr<WebSocket> websocket_;
    EventCallback event_callback_;
    AudioCallback audio_callback_;
    bool started_ = false;
};
