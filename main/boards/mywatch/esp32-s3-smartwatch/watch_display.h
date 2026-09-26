#ifndef _MYWATCH_WATCH_DISPLAY_H_
#define _MYWATCH_WATCH_DISPLAY_H_

#include "display/lcd_display.h"

#include <lvgl.h>

class WatchDisplay : public SpiLcdDisplay {
public:
    WatchDisplay(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle,
                 int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                 bool swap_xy);
    ~WatchDisplay() override;

    void SetupUI() override;
    void SetStatus(const char* status) override;
    void ShowNotification(const char* notification, int duration_ms = 3000) override;

private:
    lv_obj_t* watch_face_ = nullptr;
    lv_obj_t* brand_label_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* date_label_ = nullptr;
    lv_obj_t* assistant_button_ = nullptr;
    lv_obj_t* assistant_label_ = nullptr;
    lv_obj_t* hint_label_ = nullptr;
    lv_timer_t* clock_timer_ = nullptr;
    bool watch_face_visible_ = false;
    bool status_bar_was_hidden_ = false;
    bool emoji_box_was_hidden_ = false;
    bool preview_image_was_hidden_ = true;
    bool bottom_bar_was_hidden_ = true;

    static void RounderEventCallback(lv_event_t* event);
    static void ClockTimerCallback(lv_timer_t* timer);
    static void AssistantButtonCallback(lv_event_t* event);

    void CreateWatchFace();
    void UpdateClock();
    void UpdateWatchFaceVisibility(const char* status);
    void SetWatchFaceVisible(bool visible);
    void ApplyWatchFaceVisibility(bool visible);
    void RestoreObjectVisibility(lv_obj_t* object, bool was_hidden);
    bool IsIdleFaceStatus(const char* status) const;
};

#endif  // _MYWATCH_WATCH_DISPLAY_H_
