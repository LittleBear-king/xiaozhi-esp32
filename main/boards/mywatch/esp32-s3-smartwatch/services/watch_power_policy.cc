#include "watch_power_policy.h"

#include "backlight.h"
#include "display.h"
#include "power_save_timer.h"
#include "watch_motion_service.h"

#include <esp_log.h>

#include <utility>

#define TAG "WatchPowerPolicy"

WatchPowerPolicy::WatchPowerPolicy(Display& display, Backlight& backlight, Config config,
                                   ShutdownCallback on_shutdown)
    : display_(display),
      backlight_(backlight),
      config_(config),
      on_shutdown_(std::move(on_shutdown)),
      timer_(std::make_unique<PowerSaveTimer>(-1, config.screen_timeout_seconds,
                                              config.shutdown_timeout_seconds)) {
    timer_->OnEnterSleepMode([this]() { EnterDisplaySleep(); });
    timer_->OnExitSleepMode([this]() { ExitDisplaySleep(); });
    timer_->OnShutdownRequest([this]() {
        if (on_shutdown_) {
            on_shutdown_();
        }
    });
}

WatchPowerPolicy::~WatchPowerPolicy() {
    if (motion_service_ != nullptr) {
        motion_service_->SetEnabled(false);
    }
}

void WatchPowerPolicy::Start(bool discharging) {
    if (started_) {
        UpdatePowerSource(discharging);
        return;
    }

    started_ = true;
    discharging_ = discharging;
    timer_->SetEnabled(discharging_);
    ESP_LOGI(TAG, "Power policy started, discharging=%d", discharging_);
}

void WatchPowerPolicy::UpdatePowerSource(bool discharging) {
    if (!started_) {
        Start(discharging);
        return;
    }
    if (discharging_ == discharging) {
        return;
    }

    discharging_ = discharging;
    timer_->SetEnabled(discharging_);
    ESP_LOGI(TAG, "Power source changed, discharging=%d", discharging_);
}

void WatchPowerPolicy::AttachMotionService(WatchMotionService* motion_service) {
    if (motion_service_ == motion_service) {
        return;
    }
    if (motion_service_ != nullptr) {
        motion_service_->SetEnabled(false);
    }

    motion_service_ = motion_service;
    if (motion_service_ != nullptr) {
        motion_service_->SetEnabled(display_sleeping_.load());
    }
}

void WatchPowerPolicy::WakeDisplay() {
    if (started_) {
        timer_->WakeUp();
    }
}

void WatchPowerPolicy::EnterDisplaySleep() {
    display_sleeping_.store(true);
    if (motion_service_ != nullptr) {
        motion_service_->SetEnabled(true);
    }
    display_.SetPowerSaveMode(true);
    backlight_.SetBrightness(config_.sleeping_brightness);
    ESP_LOGI(TAG, "Display entered sleep");
}

void WatchPowerPolicy::ExitDisplaySleep() {
    display_sleeping_.store(false);
    if (motion_service_ != nullptr) {
        motion_service_->SetEnabled(false);
    }
    display_.SetPowerSaveMode(false);
    backlight_.RestoreBrightness();
    ESP_LOGI(TAG, "Display woke up");
}
