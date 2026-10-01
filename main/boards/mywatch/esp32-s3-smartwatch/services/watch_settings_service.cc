#include "watch_settings_service.h"

#include "settings.h"

#include <algorithm>
#include <utility>

WatchSettingsService::WatchSettingsService() {
    Settings settings("watch", false);
    settings_.raise_to_wake = settings.GetBool("raise_wake", true);
    settings_.do_not_disturb = settings.GetBool("dnd", false);
    settings_.use_24_hour = settings.GetBool("hour24", true);
    settings_.brightness = static_cast<uint8_t>(std::clamp(
        settings.GetInt("brightness", 75), static_cast<int32_t>(10), static_cast<int32_t>(100)));
    settings_.alarm_enabled = settings.GetBool("alarm_enabled", false);
    const auto alarm_hour = settings.GetInt("alarm_hour", 7);
    const auto alarm_minute = settings.GetInt("alarm_minute", 30);
    settings_.alarm_hour = static_cast<uint8_t>(alarm_hour < 0 ? 0 : alarm_hour > 23 ? 23 : alarm_hour);
    settings_.alarm_minute = static_cast<uint8_t>(alarm_minute < 0 ? 0 : alarm_minute > 59 ? 59 : alarm_minute);
}

void WatchSettingsService::SetChangedCallback(ChangedCallback callback) {
    changed_callback_ = std::move(callback);
    NotifyChanged();
}

void WatchSettingsService::SetRaiseToWake(bool enabled) {
    if (settings_.raise_to_wake == enabled)
        return;
    settings_.raise_to_wake = enabled;
    Save();
}

void WatchSettingsService::SetDoNotDisturb(bool enabled) {
    if (settings_.do_not_disturb == enabled)
        return;
    settings_.do_not_disturb = enabled;
    Save();
}

void WatchSettingsService::SetUse24Hour(bool enabled) {
    if (settings_.use_24_hour == enabled)
        return;
    settings_.use_24_hour = enabled;
    Save();
}

void WatchSettingsService::SetBrightness(uint8_t brightness) {
    brightness = std::clamp<uint8_t>(brightness, 10, 100);
    if (settings_.brightness == brightness)
        return;
    settings_.brightness = brightness;
    Save();
}

void WatchSettingsService::SetAlarmEnabled(bool enabled) {
    if (settings_.alarm_enabled == enabled)
        return;
    settings_.alarm_enabled = enabled;
    Save();
}

void WatchSettingsService::SetAlarmTime(uint8_t hour, uint8_t minute) {
    settings_.alarm_hour = hour % 24;
    settings_.alarm_minute = minute % 60;
    Save();
}

void WatchSettingsService::Save() {
    Settings settings("watch", true);
    settings.SetBool("raise_wake", settings_.raise_to_wake);
    settings.SetBool("dnd", settings_.do_not_disturb);
    settings.SetBool("hour24", settings_.use_24_hour);
    settings.SetInt("brightness", settings_.brightness);
    settings.SetBool("alarm_enabled", settings_.alarm_enabled);
    settings.SetInt("alarm_hour", settings_.alarm_hour);
    settings.SetInt("alarm_minute", settings_.alarm_minute);
    NotifyChanged();
}

void WatchSettingsService::NotifyChanged() {
    if (changed_callback_) {
        changed_callback_(settings_);
    }
}
