#ifndef _MYWATCH_WATCH_POWER_POLICY_H_
#define _MYWATCH_WATCH_POWER_POLICY_H_

#include <atomic>
#include <functional>
#include <memory>

class Backlight;
class Display;
class PowerSaveTimer;
class WatchMotionService;

class WatchPowerPolicy final {
public:
    struct Config {
        int screen_timeout_seconds = 60;
        int shutdown_timeout_seconds = -1;
        int sleeping_brightness = 0;
    };

    using ShutdownCallback = std::function<void()>;

    WatchPowerPolicy(Display& display, Backlight& backlight, Config config,
                     ShutdownCallback on_shutdown);
    ~WatchPowerPolicy();

    void Start(bool discharging);
    void UpdatePowerSource(bool discharging);
    void AttachMotionService(WatchMotionService* motion_service);
    void WakeDisplay();
    bool IsDisplaySleeping() const { return display_sleeping_.load(); }

private:
    void EnterDisplaySleep();
    void ExitDisplaySleep();

    Display& display_;
    Backlight& backlight_;
    Config config_;
    ShutdownCallback on_shutdown_;
    std::unique_ptr<PowerSaveTimer> timer_;
    WatchMotionService* motion_service_ = nullptr;
    std::atomic<bool> display_sleeping_{false};
    bool started_ = false;
    bool discharging_ = false;
};

#endif  // _MYWATCH_WATCH_POWER_POLICY_H_
