#ifndef _MYWATCH_WATCH_FACE_H_
#define _MYWATCH_WATCH_FACE_H_

#include "model/watch_model.h"
#include "services/watch_settings_service.h"

#include <lvgl.h>

class WatchFace final {
public:
    struct Callbacks {
        void (*on_talk)(void* context) = nullptr;
        void (*on_apps)(void* context) = nullptr;
        void (*on_tick)(void* context) = nullptr;
        void* context = nullptr;
    };

    WatchFace(lv_obj_t* parent, WatchModel& model, WatchSettingsService& settings,
              const Callbacks& callbacks);
    ~WatchFace();

    WatchFace(const WatchFace&) = delete;
    WatchFace& operator=(const WatchFace&) = delete;

    void SetVisible(bool visible);
    bool IsVisible() const { return visible_; }
    void Refresh(bool force = false);

private:
    WatchModel& model_;
    WatchSettingsService& settings_;
    lv_obj_t* root_ = nullptr;
    lv_obj_t* network_label_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* date_label_ = nullptr;
    lv_timer_t* refresh_timer_ = nullptr;
    Callbacks callbacks_;
    bool visible_ = false;
    bool has_status_snapshot_ = false;
    int last_minute_ = -1;
    int last_day_ = -1;
    int last_battery_percent_ = -1;
    bool last_charging_ = false;
    WatchNetworkState last_network_state_ = WatchNetworkState::kOffline;

    static void RefreshTimerCallback(lv_timer_t* timer);
    static void TalkButtonCallback(lv_event_t* event);
    static void AppsButtonCallback(lv_event_t* event);

    void Create(lv_obj_t* parent);
    void RefreshTime(bool force);
    void RefreshStatus(bool force);
};

#endif  // _MYWATCH_WATCH_FACE_H_
