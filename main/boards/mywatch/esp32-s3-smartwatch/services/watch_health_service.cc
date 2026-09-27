#include "watch_health_service.h"

#include "settings.h"

#include <cmath>
#include <ctime>

namespace {
constexpr float kGravityFilterAlpha = 0.08f;
constexpr float kStepTriggerMg = 145;
constexpr float kStepReleaseMg = 65;
constexpr int64_t kMinimumStepIntervalUs = 260000;
constexpr int64_t kMaximumStepIntervalUs = 1800000;
constexpr uint32_t kPersistIntervalSteps = 128;
constexpr uint32_t kStrideCentimeters = 70;
constexpr uint32_t kCaloriesHundredthsPerStep = 4;
}  // namespace

WatchHealthService::WatchHealthService() {
    Settings settings("watch_health", false);
    steps_.store(static_cast<uint32_t>(settings.GetInt("steps", 0)));
    current_day_ = settings.GetInt("day", -1);
    last_persisted_steps_ = steps_.load();
    ResetForNewDayIfNeeded();
}

void WatchHealthService::SetSensorAvailable(bool available) {
    sensor_available_.store(available, std::memory_order_relaxed);
}

void WatchHealthService::ProcessAcceleration(const WatchAcceleration& acceleration,
                                             int64_t now_us) {
    ResetForNewDayIfNeeded();
    const float magnitude =
        std::sqrt(acceleration.x_mg * acceleration.x_mg + acceleration.y_mg * acceleration.y_mg +
                  acceleration.z_mg * acceleration.z_mg);
    if (magnitude < 500 || magnitude > 2200) {
        return;
    }
    if (!filter_ready_) {
        gravity_mg_ = magnitude;
        filter_ready_ = true;
        return;
    }

    gravity_mg_ += kGravityFilterAlpha * (magnitude - gravity_mg_);
    const float dynamic_mg = magnitude - gravity_mg_;
    const int64_t interval_us = now_us - last_step_us_;
    if (dynamic_mg < kStepReleaseMg) {
        step_armed_ = true;
    }
    if (!step_armed_ || dynamic_mg < kStepTriggerMg || interval_us < kMinimumStepIntervalUs) {
        return;
    }

    if (last_step_us_ != 0 && interval_us > kMaximumStepIntervalUs) {
        // The first peak starts a walking sequence and is still a valid step.
    }
    step_armed_ = false;
    last_step_us_ = now_us;
    const uint32_t steps = steps_.fetch_add(1, std::memory_order_relaxed) + 1;
    PersistIfNeeded(steps);
}

WatchHealthSnapshot WatchHealthService::GetSnapshot() const {
    const uint32_t steps = steps_.load(std::memory_order_relaxed);
    return {
        .steps = steps,
        .distance_m = steps * kStrideCentimeters / 100,
        .calories_tenths = steps * kCaloriesHundredthsPerStep / 10,
        .active_minutes = steps / 100,
        .sensor_available = sensor_available_.load(std::memory_order_relaxed),
    };
}

void WatchHealthService::ResetToday() {
    steps_.store(0, std::memory_order_relaxed);
    last_persisted_steps_ = 0;
    Settings settings("watch_health", true);
    settings.SetInt("steps", 0);
    settings.SetInt("day", current_day_);
}

void WatchHealthService::ResetForNewDayIfNeeded() {
    const time_t now = time(nullptr);
    struct tm local = {};
    localtime_r(&now, &local);
    if (local.tm_year < 125) {
        return;
    }
    const int day_key = (local.tm_year + 1900) * 1000 + local.tm_yday;
    if (current_day_ == day_key) {
        return;
    }
    current_day_ = day_key;
    ResetToday();
}

void WatchHealthService::PersistIfNeeded(uint32_t steps) {
    if (steps - last_persisted_steps_ < kPersistIntervalSteps) {
        return;
    }
    last_persisted_steps_ = steps;
    Settings settings("watch_health", true);
    settings.SetInt("steps", static_cast<int32_t>(steps));
    settings.SetInt("day", current_day_);
}
