#include "watch_launcher_app.h"

void WatchLauncherApp::BuildContent(lv_obj_t* content) {
    AddButton(content, "运动", 0, 15, 168, 92, ActivityCallback, this);
    AddButton(content, "通知", 182, 15, 168, 92, NotificationsCallback, this);
    AddButton(content, "设置", 0, 122, 168, 92, SettingsCallback, this);
    AddButton(content, "工具", 182, 122, 168, 92, ToolsCallback, this);
    AddButton(content, "AI", 0, 229, 168, 92, AssistantCallback, this);
    AddButton(content, "表盘", 182, 229, 168, 92, HomeCallback, this);
}

void WatchLauncherApp::ActivityCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))
        ->Navigator()
        .NavigateTo(WatchAppId::kActivity);
}
void WatchLauncherApp::NotificationsCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))
        ->Navigator()
        .NavigateTo(WatchAppId::kNotifications);
}
void WatchLauncherApp::SettingsCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))
        ->Navigator()
        .NavigateTo(WatchAppId::kSettings);
}
void WatchLauncherApp::ToolsCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))
        ->Navigator()
        .NavigateTo(WatchAppId::kTools);
}
void WatchLauncherApp::AssistantCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))->Navigator().OpenAssistant();
}
void WatchLauncherApp::HomeCallback(lv_event_t* event) {
    static_cast<WatchLauncherApp*>(lv_event_get_user_data(event))->Navigator().CloseApps();
}
