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

void WatchSettingsService::Save() {
    Settings settings("watch", true);
    settings.SetBool("raise_wake", settings_.raise_to_wake);
    settings.SetBool("dnd", settings_.do_not_disturb);
    settings.SetBool("hour24", settings_.use_24_hour);
    settings.SetInt("brightness", settings_.brightness);
    NotifyChanged();
}

void WatchSettingsService::NotifyChanged() {
    if (changed_callback_) {
        changed_callback_(settings_);
    }
}
