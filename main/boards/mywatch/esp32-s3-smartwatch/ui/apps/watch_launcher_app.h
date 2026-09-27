#ifndef _MYWATCH_WATCH_LAUNCHER_APP_H_
#define _MYWATCH_WATCH_LAUNCHER_APP_H_

#include "watch_app_base.h"

class WatchLauncherApp final : public WatchAppBase {
public:
    WatchLauncherApp() : WatchAppBase("应用") {}
    WatchAppId Id() const override { return WatchAppId::kLauncher; }

protected:
    void BuildContent(lv_obj_t* content) override;

private:
    static void ActivityCallback(lv_event_t* event);
    static void NotificationsCallback(lv_event_t* event);
    static void SettingsCallback(lv_event_t* event);
    static void ToolsCallback(lv_event_t* event);
    static void AssistantCallback(lv_event_t* event);
    static void HomeCallback(lv_event_t* event);
};

#endif  // _MYWATCH_WATCH_LAUNCHER_APP_H_
