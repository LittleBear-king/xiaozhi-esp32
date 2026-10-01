#ifndef _MYWATCH_WATCH_TIME_SERVICE_H_
#define _MYWATCH_WATCH_TIME_SERVICE_H_

#include "hal/watch_rtc.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <atomic>
#include <functional>

struct WatchTimeSnapshot {
    bool rtc_available = false;
    bool system_time_valid = false;
    bool rtc_synchronized = false;
};

class WatchTimeService final {
public:
    explicit WatchTimeService(WatchRtc& rtc) : rtc_(rtc) {}
    ~WatchTimeService();

    bool Start();
    void Stop();
    void SetAlarmProvider(std::function<bool(int, int)> provider,
                         std::function<void()> callback);
    WatchTimeSnapshot GetSnapshot() const;

private:
    static void TaskEntry(void* context);
    void TaskLoop();
    static bool IsTimeValid(time_t value);

    WatchRtc& rtc_;
    std::atomic<bool> running_{false};
    std::atomic<TaskHandle_t> task_{nullptr};
    std::atomic<bool> rtc_available_{false};
    std::atomic<bool> system_time_valid_{false};
    std::atomic<bool> rtc_synchronized_{false};
    std::function<bool(int, int)> alarm_provider_;
    std::function<void()> alarm_callback_;
    int last_alarm_minute_ = -1;
};

#endif  // _MYWATCH_WATCH_TIME_SERVICE_H_
