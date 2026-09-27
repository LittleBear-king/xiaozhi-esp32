#include "watch_notification_service.h"

#include <algorithm>
#include <cstdio>
#include <ctime>

void WatchNotificationService::Push(const char* source, const char* title, const char* body) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& item = items_[next_];
    item = {};
    item.id = next_id_++;
    item.timestamp = static_cast<int64_t>(time(nullptr));
    item.unread = true;
    snprintf(item.source.data(), item.source.size(), "%s", source == nullptr ? "Watch" : source);
    snprintf(item.title.data(), item.title.size(), "%s", title == nullptr ? "Notification" : title);
    snprintf(item.body.data(), item.body.size(), "%s", body == nullptr ? "" : body);
    next_ = (next_ + 1) % items_.size();
    count_ = std::min(count_ + 1, items_.size());
}

WatchNotificationSnapshot WatchNotificationService::GetSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    WatchNotificationSnapshot snapshot;
    snapshot.count = count_;
    for (size_t offset = 0; offset < count_; ++offset) {
        const size_t index = (next_ + items_.size() - 1 - offset) % items_.size();
        snapshot.items[offset] = items_[index];
        if (snapshot.items[offset].unread) {
            ++snapshot.unread;
        }
    }
    return snapshot;
}

void WatchNotificationService::MarkAllRead() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& item : items_) {
        item.unread = false;
    }
}

void WatchNotificationService::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    items_ = {};
    count_ = 0;
    next_ = 0;
}
