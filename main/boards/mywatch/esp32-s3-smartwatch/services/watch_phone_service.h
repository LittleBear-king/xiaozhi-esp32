#ifndef _MYWATCH_WATCH_PHONE_SERVICE_H_
#define _MYWATCH_WATCH_PHONE_SERVICE_H_

#include "watch_notification_service.h"

#include <atomic>
#include <cstddef>
#include <string>

struct WatchPhoneSnapshot {
    bool connected = false;
    uint32_t received_messages = 0;
    uint32_t rejected_messages = 0;
};

class WatchPhoneService final {
public:
    explicit WatchPhoneService(WatchNotificationService& notifications)
        : notifications_(notifications) {}

    void SetConnected(bool connected);
    bool HandleMessage(const char* data, size_t length);
    WatchPhoneSnapshot GetSnapshot() const;

private:
    WatchNotificationService& notifications_;
    std::atomic<bool> connected_{false};
    std::atomic<uint32_t> received_messages_{0};
    std::atomic<uint32_t> rejected_messages_{0};
};

#endif  // _MYWATCH_WATCH_PHONE_SERVICE_H_
