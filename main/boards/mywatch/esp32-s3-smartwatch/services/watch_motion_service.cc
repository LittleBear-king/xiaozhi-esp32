#include "watch_motion_service.h"

#include <esp_log.h>
#include <esp_timer.h>

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

WatchMotionService::WatchMotionService(WatchMotion& motion, Config config, RaiseCallback on_raise)
    : motion_(motion), config_(config), on_raise_(std::move(on_raise)) {}

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
    enabled_.store(enabled);
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
        if (!enabled_.load()) {
            filter_ready_ = false;
            raise_armed_ = false;
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            continue;
        }

        WatchAcceleration acceleration;
        if (motion_.ReadAcceleration(acceleration)) {
            ProcessSample(acceleration, esp_timer_get_time());
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

    const float face_up_z_mg = acceleration.z_mg * config_.face_up_z_sign;
    if (!filter_ready_) {
        filtered_z_mg_ = face_up_z_mg;
        filter_ready_ = true;
    } else {
        filtered_z_mg_ += kFilterAlpha * (face_up_z_mg - filtered_z_mg_);
    }

    if (now_us < cooldown_until_us_) {
        return;
    }

    if (!raise_armed_) {
        if (filtered_z_mg_ <= config_.lowered_z_max_mg) {
            raise_armed_ = true;
            armed_z_mg_ = filtered_z_mg_;
            armed_at_us_ = now_us;
        }
        return;
    }

    const int64_t raise_window_us = static_cast<int64_t>(config_.raise_window_ms) * 1000;
    if (now_us - armed_at_us_ > raise_window_us) {
        raise_armed_ = false;
        return;
    }

    if (filtered_z_mg_ >= config_.raised_z_min_mg &&
        filtered_z_mg_ - armed_z_mg_ >= config_.minimum_raise_delta_mg) {
        raise_armed_ = false;
        cooldown_until_us_ = now_us + static_cast<int64_t>(config_.cooldown_ms) * 1000;
        ESP_LOGI(TAG, "Wrist raise detected, z=%.0f mg", static_cast<double>(filtered_z_mg_));
        if (on_raise_) {
            on_raise_();
        }
    }
}
