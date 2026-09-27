#ifndef _MYWATCH_WATCH_NOTIFICATION_SERVICE_H_
#define _MYWATCH_WATCH_NOTIFICATION_SERVICE_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>

struct WatchNotification {
    uint32_t id = 0;
    int64_t timestamp = 0;
    bool unread = false;
    std::array<char, 24> source{};
    std::array<char, 96> title{};
    std::array<char, 160> body{};
};

struct WatchNotificationSnapshot {
    static constexpr size_t kCapacity = 12;
    std::array<WatchNotification, kCapacity> items{};
    size_t count = 0;
    size_t unread = 0;
};

class WatchNotificationService final {
public:
    void Push(const char* source, const char* title, const char* body);
    WatchNotificationSnapshot GetSnapshot() const;
    void MarkAllRead();
    void Clear();

private:
    mutable std::mutex mutex_;
    std::array<WatchNotification, WatchNotificationSnapshot::kCapacity> items_{};
    size_t count_ = 0;
    size_t next_ = 0;
    uint32_t next_id_ = 1;
};

#endif  // _MYWATCH_WATCH_NOTIFICATION_SERVICE_H_
