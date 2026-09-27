#ifndef _MYWATCH_WATCH_HEALTH_SERVICE_H_
#define _MYWATCH_WATCH_HEALTH_SERVICE_H_

#include "hal/watch_motion.h"

#include <atomic>
#include <cstdint>

struct WatchHealthSnapshot {
    uint32_t steps = 0;
    uint32_t distance_m = 0;
    uint32_t calories_tenths = 0;
    uint32_t active_minutes = 0;
    bool sensor_available = false;
};

class WatchHealthService final {
public:
    WatchHealthService();

    void SetSensorAvailable(bool available);
    void ProcessAcceleration(const WatchAcceleration& acceleration, int64_t now_us);
    WatchHealthSnapshot GetSnapshot() const;
    void ResetToday();

private:
    void ResetForNewDayIfNeeded();
    void PersistIfNeeded(uint32_t steps);

    std::atomic<uint32_t> steps_{0};
    std::atomic<bool> sensor_available_{false};
    float gravity_mg_ = 1000;
    bool filter_ready_ = false;
    bool step_armed_ = true;
    int64_t last_step_us_ = 0;
    int current_day_ = -1;
    uint32_t last_persisted_steps_ = 0;
};

#endif  // _MYWATCH_WATCH_HEALTH_SERVICE_H_
