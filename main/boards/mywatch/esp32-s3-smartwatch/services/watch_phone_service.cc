#include "watch_phone_service.h"

#include <cJSON.h>

#include <cstring>
#include <memory>

namespace {
constexpr size_t kMaximumMessageBytes = 768;
constexpr size_t kMaximumSourceBytes = 23;
constexpr size_t kMaximumTitleBytes = 95;
constexpr size_t kMaximumBodyBytes = 159;

bool IsBoundedString(const cJSON* value, size_t maximum) {
    return cJSON_IsString(value) && value->valuestring != nullptr &&
           strnlen(value->valuestring, maximum + 1) <= maximum;
}
}  // namespace

void WatchPhoneService::SetConnected(bool connected) {
    connected_.store(connected, std::memory_order_relaxed);
}

bool WatchPhoneService::HandleMessage(const char* data, size_t length) {
    if (data == nullptr || length == 0 || length > kMaximumMessageBytes) {
        rejected_messages_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    std::unique_ptr<cJSON, decltype(&cJSON_Delete)> root(cJSON_ParseWithLength(data, length),
                                                         cJSON_Delete);
    const cJSON* type = root ? cJSON_GetObjectItemCaseSensitive(root.get(), "type") : nullptr;
    if (!IsBoundedString(type, 32) || strcmp(type->valuestring, "notification") != 0) {
        rejected_messages_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    const cJSON* source = cJSON_GetObjectItemCaseSensitive(root.get(), "source");
    const cJSON* title = cJSON_GetObjectItemCaseSensitive(root.get(), "title");
    const cJSON* body = cJSON_GetObjectItemCaseSensitive(root.get(), "body");
    if (!IsBoundedString(source, kMaximumSourceBytes) ||
        !IsBoundedString(title, kMaximumTitleBytes) || !IsBoundedString(body, kMaximumBodyBytes)) {
        rejected_messages_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    notifications_.Push(source->valuestring, title->valuestring, body->valuestring);
    received_messages_.fetch_add(1, std::memory_order_relaxed);
    return true;
}

WatchPhoneSnapshot WatchPhoneService::GetSnapshot() const {
    return {
        .connected = connected_.load(std::memory_order_relaxed),
        .received_messages = received_messages_.load(std::memory_order_relaxed),
        .rejected_messages = rejected_messages_.load(std::memory_order_relaxed),
    };
}
