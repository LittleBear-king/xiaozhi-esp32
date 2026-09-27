#ifndef _MYWATCH_WATCH_MOTION_SERVICE_H_
#define _MYWATCH_WATCH_MOTION_SERVICE_H_

#include "hal/watch_motion.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <atomic>
#include <cstdint>
#include <functional>

class WatchMotionService final {
public:
    struct Config {
        float lowered_normal_max_mg = 420;
        float raised_normal_min_mg = 650;
        float minimum_raise_delta_mg = 250;
        int raise_window_ms = 3500;
        int raised_stable_samples = 2;
        int cooldown_ms = 3000;
        int sample_period_ms = 100;
    };

    using RaiseCallback = std::function<void()>;
    using SampleCallback = std::function<void(const WatchAcceleration&, int64_t)>;

    WatchMotionService(WatchMotion& motion, Config config, RaiseCallback on_raise,
                       SampleCallback on_sample = {});
    ~WatchMotionService();

    bool Start();
    void Stop();
    void SetEnabled(bool enabled);

private:
    static void TaskEntry(void* context);
    void TaskLoop();
    void ProcessSample(const WatchAcceleration& acceleration, int64_t now_us);

    WatchMotion& motion_;
    Config config_;
    RaiseCallback on_raise_;
    SampleCallback on_sample_;
    std::atomic<bool> running_{false};
    std::atomic<bool> raise_enabled_{false};
    std::atomic<bool> reset_detection_requested_{true};
    std::atomic<TaskHandle_t> task_{nullptr};
    bool filter_ready_ = false;
    bool raise_armed_ = false;
    int raised_stable_count_ = 0;
    float filtered_normal_mg_ = 0;
    float armed_normal_mg_ = 0;
    int64_t armed_at_us_ = 0;
    int64_t cooldown_until_us_ = 0;
};

#endif  // _MYWATCH_WATCH_MOTION_SERVICE_H_
