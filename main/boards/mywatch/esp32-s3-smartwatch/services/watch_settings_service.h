#ifndef _MYWATCH_WATCH_SETTINGS_SERVICE_H_
#define _MYWATCH_WATCH_SETTINGS_SERVICE_H_

#include <cstdint>
#include <functional>

struct WatchSettingsSnapshot {
    bool raise_to_wake = true;
    bool do_not_disturb = false;
    bool use_24_hour = true;
    uint8_t brightness = 75;
    bool alarm_enabled = false;
    uint8_t alarm_hour = 7;
    uint8_t alarm_minute = 30;
};

class WatchSettingsService final {
public:
    using ChangedCallback = std::function<void(const WatchSettingsSnapshot&)>;

    WatchSettingsService();

    WatchSettingsSnapshot GetSnapshot() const { return settings_; }
    void SetChangedCallback(ChangedCallback callback);
    void SetRaiseToWake(bool enabled);
    void SetDoNotDisturb(bool enabled);
    void SetUse24Hour(bool enabled);
    void SetBrightness(uint8_t brightness);
    void SetAlarmEnabled(bool enabled);
    void SetAlarmTime(uint8_t hour, uint8_t minute);

private:
    void Save();
    void NotifyChanged();

    WatchSettingsSnapshot settings_;
    ChangedCallback changed_callback_;
};

#endif  // _MYWATCH_WATCH_SETTINGS_SERVICE_H_
