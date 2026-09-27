#include "watch_motion_service.h"

#include <esp_log.h>
#include <esp_timer.h>

#include <algorithm>
#include <cmath>
#include <utility>

#define TAG "WatchMotionService"

namespace {
constexpr float kFilterAlpha = 0.35f;
constexpr float kMinimumGravityMg = 650;
constexpr float kMaximumGravityMg = 1350;
constexpr uint32_t kTaskStackSize = 3072;
constexpr UBaseType_t kTaskPriority = 2;
}  // namespace

WatchMotionService::WatchMotionService(WatchMotion& motion, Config config, RaiseCallback on_raise,
                                       SampleCallback on_sample)
    : motion_(motion),
      config_(config),
      on_raise_(std::move(on_raise)),
      on_sample_(std::move(on_sample)) {}

WatchMotionService::~WatchMotionService() { Stop(); }

bool WatchMotionService::Start() {
    if (running_.exchange(true)) {
        return true;
    }

    TaskHandle_t task = nullptr;
    const BaseType_t result =
        xTaskCreate(TaskEntry, "watch_motion", kTaskStackSize, this, kTaskPriority, &task);
    if (result != pdPASS) {
        running_.store(false);
        ESP_LOGE(TAG, "Failed to create motion task");
        return false;
    }
    task_.store(task);
    return true;
}

void WatchMotionService::Stop() {
    running_.store(false);
    TaskHandle_t task = task_.load();
    if (task == nullptr) {
        return;
    }

    xTaskNotifyGive(task);
    while (task_.load() != nullptr) {
        vTaskDelay(1);
    }
}

void WatchMotionService::SetEnabled(bool enabled) {
    if (raise_enabled_.exchange(enabled) == enabled) {
        return;
    }
    reset_detection_requested_.store(true);
    ESP_LOGI(TAG, "Raise-to-wake detection %s", enabled ? "enabled" : "disabled");
    TaskHandle_t task = task_.load();
    if (task != nullptr) {
        xTaskNotifyGive(task);
    }
}

void WatchMotionService::TaskEntry(void* context) {
    static_cast<WatchMotionService*>(context)->TaskLoop();
}

void WatchMotionService::TaskLoop() {
    ESP_LOGI(TAG, "Raise-to-wake detection started");
    while (running_.load()) {
        WatchAcceleration acceleration;
        if (motion_.ReadAcceleration(acceleration)) {
            const int64_t now_us = esp_timer_get_time();
            if (on_sample_) {
                on_sample_(acceleration, now_us);
            }
            if (raise_enabled_.load()) {
                if (reset_detection_requested_.exchange(false)) {
                    filter_ready_ = false;
                    raise_armed_ = false;
                    raised_stable_count_ = 0;
                }
                ProcessSample(acceleration, now_us);
            } else {
                filter_ready_ = false;
                raise_armed_ = false;
                raised_stable_count_ = 0;
                reset_detection_requested_.store(false);
            }
        }
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(config_.sample_period_ms));
    }

    task_.store(nullptr);
    vTaskDelete(nullptr);
}

void WatchMotionService::ProcessSample(const WatchAcceleration& acceleration, int64_t now_us) {
    const float gravity_mg =
        std::sqrt(acceleration.x_mg * acceleration.x_mg + acceleration.y_mg * acceleration.y_mg +
                  acceleration.z_mg * acceleration.z_mg);
    if (gravity_mg < kMinimumGravityMg || gravity_mg > kMaximumGravityMg) {
        return;
    }

    // A hanging wrist is near 0 mg on the screen-normal axis; a raised screen is
    // much closer to 1 g. Magnitude makes this independent of the sensor Z polarity.
    const float screen_normal_mg = std::abs(acceleration.z_mg);
    if (!filter_ready_) {
        filtered_normal_mg_ = screen_normal_mg;
        filter_ready_ = true;
    } else {
        filtered_normal_mg_ += kFilterAlpha * (screen_normal_mg - filtered_normal_mg_);
    }

    if (now_us < cooldown_until_us_) {
        return;
    }

    if (!raise_armed_) {
        if (filtered_normal_mg_ <= config_.lowered_normal_max_mg) {
            raise_armed_ = true;
            armed_normal_mg_ = filtered_normal_mg_;
            armed_at_us_ = now_us;
            raised_stable_count_ = 0;
        }
        return;
    }

    armed_normal_mg_ = std::min(armed_normal_mg_, filtered_normal_mg_);

    const int64_t raise_window_us = static_cast<int64_t>(config_.raise_window_ms) * 1000;
    if (now_us - armed_at_us_ > raise_window_us) {
        raise_armed_ = false;
        raised_stable_count_ = 0;
        return;
    }

    const bool raised = filtered_normal_mg_ >= config_.raised_normal_min_mg &&
                        filtered_normal_mg_ - armed_normal_mg_ >= config_.minimum_raise_delta_mg;
    raised_stable_count_ = raised ? raised_stable_count_ + 1 : 0;
    if (raised_stable_count_ >= config_.raised_stable_samples) {
        raise_armed_ = false;
        raised_stable_count_ = 0;
        cooldown_until_us_ = now_us + static_cast<int64_t>(config_.cooldown_ms) * 1000;
        ESP_LOGI(TAG, "Wrist raise detected, normal=%.0f mg",
                 static_cast<double>(filtered_normal_mg_));
        if (on_raise_) {
            on_raise_();
        }
    }
}
