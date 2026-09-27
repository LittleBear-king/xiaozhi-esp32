#ifndef _MYWATCH_WATCH_DISPLAY_H_
#define _MYWATCH_WATCH_DISPLAY_H_

#include "display/lcd_display.h"

#include <lvgl.h>

#include <memory>

class WatchController;
class WatchFace;
class WatchModel;
class WatchAppRouter;
class WatchHealthService;
class WatchNotificationService;
class WatchPhoneService;
class WatchReliabilityService;
class WatchSettingsService;
class WatchTimeService;

class WatchDisplay final : public SpiLcdDisplay {
public:
    WatchDisplay(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle,
                 int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                 bool swap_xy, WatchModel& model, WatchController& controller,
                 WatchHealthService& health, WatchNotificationService& notifications,
                 WatchSettingsService& settings, WatchTimeService& time, WatchPhoneService& phone,
                 WatchReliabilityService& reliability);
    ~WatchDisplay() override;

    void SetupUI() override;
    void SetStatus(const char* status) override;
    void ShowNotification(const char* notification, int duration_ms = 3000) override;

private:
    struct AssistantLayerVisibility {
        bool top_bar_hidden = false;
        bool status_bar_hidden = false;
        bool emoji_box_hidden = false;
        bool preview_image_hidden = true;
        bool bottom_bar_hidden = true;
    };

    WatchModel& model_;
    WatchController& controller_;
    WatchHealthService& health_;
    WatchNotificationService& notifications_;
    WatchSettingsService& settings_;
    WatchTimeService& time_;
    WatchPhoneService& phone_;
    WatchReliabilityService& reliability_;
    std::unique_ptr<WatchFace> watch_face_;
    std::unique_ptr<WatchAppRouter> app_router_;
    AssistantLayerVisibility assistant_visibility_;

    static void RounderEventCallback(lv_event_t* event);
    static void TalkRequested(void* context);
    static void AppsRequested(void* context);
    static void FaceTick(void* context);

    void HandleTalkRequested();
    void HandleAppsRequested();
    void HandleAppsClosed();
    void MaybeRestoreIdleFace();
    void UpdateWatchFaceVisibility(const char* status);
    void SetWatchFaceVisible(bool visible);
    void ApplyWatchFaceVisibility(bool visible);
    void CaptureAssistantLayerVisibility();
    void RestoreAssistantLayerVisibility();
    static bool IsObjectHidden(lv_obj_t* object);
    static void SetObjectHidden(lv_obj_t* object, bool hidden);
    bool IsIdleFaceStatus(const char* status) const;
};

#endif  // _MYWATCH_WATCH_DISPLAY_H_
