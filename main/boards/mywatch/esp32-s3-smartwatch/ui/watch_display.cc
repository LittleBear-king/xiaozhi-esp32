#include "watch_display.h"

#include "apps/watch_app_router.h"
#include "assets/lang_config.h"
#include "controller/watch_controller.h"
#include "services/watch_health_service.h"
#include "services/watch_notification_service.h"
#include "services/watch_phone_service.h"
#include "services/watch_reliability_service.h"
#include "services/watch_settings_service.h"
#include "services/watch_time_service.h"
#include "ui/apps/watch_activity_app.h"
#include "ui/apps/watch_launcher_app.h"
#include "ui/apps/watch_notifications_app.h"
#include "ui/apps/watch_settings_app.h"
#include "ui/apps/watch_tools_app.h"
#include "watch_face.h"

#include <cstring>
#include <utility>

WatchDisplay::WatchDisplay(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle,
                           int width, int height, int offset_x, int offset_y, bool mirror_x,
                           bool mirror_y, bool swap_xy, WatchModel& model,
                           WatchController& controller, WatchHealthService& health,
                           WatchNotificationService& notifications, WatchSettingsService& settings,
                           WatchTimeService& time, WatchPhoneService& phone,
                           WatchReliabilityService& reliability)
    : SpiLcdDisplay(io_handle, panel_handle, width, height, offset_x, offset_y, mirror_x, mirror_y,
                    swap_xy),
      model_(model),
      controller_(controller),
      health_(health),
      notifications_(notifications),
      settings_(settings),
      time_(time),
      phone_(phone),
      reliability_(reliability) {}

WatchDisplay::~WatchDisplay() {
    DisplayLockGuard lock(this);
    app_router_.reset();
    watch_face_.reset();
}

void WatchDisplay::RounderEventCallback(lv_event_t* event) {
    auto* area = static_cast<lv_area_t*>(lv_event_get_param(event));
    area->x1 = (area->x1 >> 1) << 1;
    area->y1 = (area->y1 >> 1) << 1;
    area->x2 = ((area->x2 >> 1) << 1) + 1;
    area->y2 = ((area->y2 >> 1) << 1) + 1;
}

void WatchDisplay::TalkRequested(void* context) {
    static_cast<WatchDisplay*>(context)->HandleTalkRequested();
}

void WatchDisplay::AppsRequested(void* context) {
    static_cast<WatchDisplay*>(context)->HandleAppsRequested();
}

void WatchDisplay::FaceTick(void* context) {
    static_cast<WatchDisplay*>(context)->MaybeRestoreIdleFace();
}

void WatchDisplay::SetupUI() {
    if (IsSetupUICalled()) {
        return;
    }

    SpiLcdDisplay::SetupUI();

    DisplayLockGuard lock(this);
    lv_obj_set_style_pad_left(status_bar_, LV_HOR_RES * 0.1, 0);
    lv_obj_set_style_pad_right(status_bar_, LV_HOR_RES * 0.1, 0);
    lv_display_add_event_cb(display_, RounderEventCallback, LV_EVENT_INVALIDATE_AREA, nullptr);

    WatchFace::Callbacks callbacks = {
        .on_talk = TalkRequested,
        .on_apps = AppsRequested,
        .on_tick = FaceTick,
        .context = this,
    };
    watch_face_ = std::make_unique<WatchFace>(lv_screen_active(), model_, settings_, callbacks);

    WatchAppRouter::Callbacks router_callbacks = {
        .on_close = [this]() { HandleAppsClosed(); },
        .on_assistant = [this]() { HandleTalkRequested(); },
    };
    app_router_ = std::make_unique<WatchAppRouter>(std::move(router_callbacks));
    app_router_->Attach(lv_screen_active());
    app_router_->Register(std::make_unique<WatchLauncherApp>());
    app_router_->Register(std::make_unique<WatchActivityApp>(health_));
    app_router_->Register(std::make_unique<WatchNotificationsApp>(notifications_));
    app_router_->Register(std::make_unique<WatchSettingsApp>(settings_));
    app_router_->Register(std::make_unique<WatchToolsApp>(time_, phone_, reliability_));
    ApplyWatchFaceVisibility(true);
}

void WatchDisplay::SetStatus(const char* status) {
    SpiLcdDisplay::SetStatus(status);
    UpdateWatchFaceVisibility(status);
}

void WatchDisplay::ShowNotification(const char* notification, int duration_ms) {
    notifications_.Push("System", "Notification", notification);
    if (settings_.GetSnapshot().do_not_disturb) {
        return;
    }
    SpiLcdDisplay::ShowNotification(notification, duration_ms);
    SetWatchFaceVisible(false);
}

void WatchDisplay::HandleTalkRequested() {
    const bool from_apps = app_router_ != nullptr && app_router_->IsVisible();
    if (app_router_ != nullptr) {
        app_router_->Hide();
    }
    if (from_apps) {
        RestoreAssistantLayerVisibility();
    } else {
        ApplyWatchFaceVisibility(false);
    }
    controller_.RequestTalk();
}

void WatchDisplay::HandleAppsRequested() {
    if (watch_face_ == nullptr || app_router_ == nullptr)
        return;
    SetObjectHidden(top_bar_, true);
    SetObjectHidden(status_bar_, true);
    SetObjectHidden(emoji_box_, true);
    SetObjectHidden(preview_image_, true);
    SetObjectHidden(bottom_bar_, true);
    watch_face_->SetVisible(false);
    app_router_->Show();
}

void WatchDisplay::HandleAppsClosed() { ApplyWatchFaceVisibility(true); }

void WatchDisplay::MaybeRestoreIdleFace() {
    if (app_router_ != nullptr) {
        app_router_->Tick();
        if (app_router_->IsVisible())
            return;
    }
    if (!controller_.IsIdle() || notification_label_ == nullptr || status_label_ == nullptr ||
        !lv_obj_has_flag(notification_label_, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }

    ApplyWatchFaceVisibility(IsIdleFaceStatus(lv_label_get_text(status_label_)));
}

bool WatchDisplay::IsIdleFaceStatus(const char* status) const {
    if (status == nullptr) {
        return false;
    }
    if (strcmp(status, Lang::Strings::STANDBY) == 0) {
        return true;
    }
    return strlen(status) == 5 && status[2] == ':' && status[0] >= '0' && status[0] <= '9' &&
           status[1] >= '0' && status[1] <= '9' && status[3] >= '0' && status[3] <= '9' &&
           status[4] >= '0' && status[4] <= '9';
}

void WatchDisplay::UpdateWatchFaceVisibility(const char* status) {
    if (status == nullptr) {
        return;
    }
    if (IsIdleFaceStatus(status)) {
        if (app_router_ == nullptr || !app_router_->IsVisible()) {
            SetWatchFaceVisible(true);
        }
        return;
    }
    if (strcmp(status, Lang::Strings::CONNECTING) == 0 ||
        strcmp(status, Lang::Strings::LISTENING) == 0 ||
        strcmp(status, Lang::Strings::SPEAKING) == 0) {
        SetWatchFaceVisible(false);
    }
}

void WatchDisplay::SetWatchFaceVisible(bool visible) {
    DisplayLockGuard lock(this);
    ApplyWatchFaceVisibility(visible);
}

void WatchDisplay::ApplyWatchFaceVisibility(bool visible) {
    if (watch_face_ == nullptr || visible == watch_face_->IsVisible()) {
        return;
    }

    if (visible) {
        const bool from_apps = app_router_ != nullptr && app_router_->IsVisible();
        if (app_router_ != nullptr)
            app_router_->Hide();
        if (!from_apps) {
            CaptureAssistantLayerVisibility();
        }
        SetObjectHidden(top_bar_, true);
        SetObjectHidden(status_bar_, true);
        SetObjectHidden(emoji_box_, true);
        SetObjectHidden(preview_image_, true);
        SetObjectHidden(bottom_bar_, true);
        watch_face_->SetVisible(true);
    } else {
        watch_face_->SetVisible(false);
        if (app_router_ != nullptr)
            app_router_->Hide();
        RestoreAssistantLayerVisibility();
    }
}

void WatchDisplay::CaptureAssistantLayerVisibility() {
    assistant_visibility_.top_bar_hidden = IsObjectHidden(top_bar_);
    assistant_visibility_.status_bar_hidden = IsObjectHidden(status_bar_);
    assistant_visibility_.emoji_box_hidden = IsObjectHidden(emoji_box_);
    assistant_visibility_.preview_image_hidden = IsObjectHidden(preview_image_);
    assistant_visibility_.bottom_bar_hidden = IsObjectHidden(bottom_bar_);
}

void WatchDisplay::RestoreAssistantLayerVisibility() {
    SetObjectHidden(top_bar_, assistant_visibility_.top_bar_hidden);
    SetObjectHidden(status_bar_, assistant_visibility_.status_bar_hidden);
    SetObjectHidden(emoji_box_, assistant_visibility_.emoji_box_hidden);
    SetObjectHidden(preview_image_, assistant_visibility_.preview_image_hidden);
    SetObjectHidden(bottom_bar_, assistant_visibility_.bottom_bar_hidden);
}

bool WatchDisplay::IsObjectHidden(lv_obj_t* object) {
    return object == nullptr || lv_obj_has_flag(object, LV_OBJ_FLAG_HIDDEN);
}

void WatchDisplay::SetObjectHidden(lv_obj_t* object, bool hidden) {
    if (object == nullptr) {
        return;
    }
    if (hidden) {
        lv_obj_add_flag(object, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(object, LV_OBJ_FLAG_HIDDEN);
    }
}
