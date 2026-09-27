#ifndef _MYWATCH_WATCH_RELIABILITY_SERVICE_H_
#define _MYWATCH_WATCH_RELIABILITY_SERVICE_H_

#include <esp_timer.h>

#include <cstdint>
#include <mutex>

struct WatchReliabilitySnapshot {
    uint32_t boot_count = 0;
    uint32_t consecutive_faults = 0;
    int reset_reason = 0;
    bool ota_pending_verification = false;
    bool task_watchdog_enabled = false;
};

class WatchReliabilityService final {
public:
    WatchReliabilityService();
    ~WatchReliabilityService();

    WatchReliabilitySnapshot GetSnapshot() const;

private:
    static void StableTimerCallback(void* context);
    void MarkStable();

    mutable std::mutex mutex_;
    WatchReliabilitySnapshot snapshot_;
    esp_timer_handle_t stable_timer_ = nullptr;
};

#endif  // _MYWATCH_WATCH_RELIABILITY_SERVICE_H_
