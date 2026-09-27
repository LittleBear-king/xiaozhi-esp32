#ifndef _MYWATCH_WATCH_FACE_H_
#define _MYWATCH_WATCH_FACE_H_

#include <lvgl.h>

class WatchFace final {
public:
    struct Callbacks {
        void (*on_talk)(void* context) = nullptr;
        void (*on_tick)(void* context) = nullptr;
        void* context = nullptr;
    };

    WatchFace(lv_obj_t* parent, const Callbacks& callbacks);
    ~WatchFace();

    WatchFace(const WatchFace&) = delete;
    WatchFace& operator=(const WatchFace&) = delete;

    void SetVisible(bool visible);
    bool IsVisible() const { return visible_; }
    void RefreshClock(bool force = false);

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* date_label_ = nullptr;
    lv_timer_t* clock_timer_ = nullptr;
    Callbacks callbacks_;
    bool visible_ = false;
    int last_minute_ = -1;
    int last_day_ = -1;

    static void ClockTimerCallback(lv_timer_t* timer);
    static void TalkButtonCallback(lv_event_t* event);

    void Create(lv_obj_t* parent);
};

#endif  // _MYWATCH_WATCH_FACE_H_
