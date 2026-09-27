#ifndef _MYWATCH_WATCH_APP_H_
#define _MYWATCH_WATCH_APP_H_

#include <lvgl.h>

#include <cstdint>

enum class WatchAppId : uint8_t {
    kLauncher,
    kActivity,
    kNotifications,
    kSettings,
    kTools,
};

class WatchAppNavigator {
public:
    virtual ~WatchAppNavigator() = default;
    virtual void NavigateTo(WatchAppId id) = 0;
    virtual void NavigateBack() = 0;
    virtual void CloseApps() = 0;
    virtual void OpenAssistant() = 0;
};

class WatchApp {
public:
    virtual ~WatchApp() = default;
    virtual WatchAppId Id() const = 0;
    virtual void Create(lv_obj_t* parent, WatchAppNavigator& navigator) = 0;
    virtual void OnResume() = 0;
    virtual void OnPause() = 0;
    virtual void OnTick() {}
};

#endif  // _MYWATCH_WATCH_APP_H_
