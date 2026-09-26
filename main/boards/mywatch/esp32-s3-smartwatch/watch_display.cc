#include "watch_display.h"

#include "application.h"
#include "assets/lang_config.h"
#include "watch_face.h"

#include <cstring>

WatchDisplay::WatchDisplay(esp_lcd_panel_io_handle_t io_handle, esp_lcd_panel_handle_t panel_handle,
                           int width, int height, int offset_x, int offset_y, bool mirror_x,
                           bool mirror_y, bool swap_xy)
    : SpiLcdDisplay(io_handle, panel_handle, width, height, offset_x, offset_y, mirror_x, mirror_y,
                    swap_xy) {}

WatchDisplay::~WatchDisplay() {
    DisplayLockGuard lock(this);
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
        .on_tick = FaceTick,
        .context = this,
    };
    watch_face_ = std::make_unique<WatchFace>(container_, callbacks);
}

void WatchDisplay::SetStatus(const char* status) {
    SpiLcdDisplay::SetStatus(status);
    UpdateWatchFaceVisibility(status);
}

void WatchDisplay::ShowNotification(const char* notification, int duration_ms) {
    SpiLcdDisplay::ShowNotification(notification, duration_ms);
    SetWatchFaceVisible(false);
}

void WatchDisplay::HandleTalkRequested() {
    ApplyWatchFaceVisibility(false);
    Application::GetInstance().ToggleChatState();
}

void WatchDisplay::MaybeRestoreIdleFace() {
    if (Application::GetInstance().GetDeviceState() != kDeviceStateIdle ||
        notification_label_ == nullptr || status_label_ == nullptr ||
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
    const bool show =
        Application::GetInstance().GetDeviceState() == kDeviceStateIdle && IsIdleFaceStatus(status);
    SetWatchFaceVisible(show);
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
        CaptureAssistantLayerVisibility();
        SetObjectHidden(status_bar_, true);
        SetObjectHidden(emoji_box_, true);
        SetObjectHidden(preview_image_, true);
        SetObjectHidden(bottom_bar_, true);
        watch_face_->SetVisible(true);
    } else {
        watch_face_->SetVisible(false);
        RestoreAssistantLayerVisibility();
    }
}

void WatchDisplay::CaptureAssistantLayerVisibility() {
    assistant_visibility_.status_bar_hidden = IsObjectHidden(status_bar_);
    assistant_visibility_.emoji_box_hidden = IsObjectHidden(emoji_box_);
    assistant_visibility_.preview_image_hidden = IsObjectHidden(preview_image_);
    assistant_visibility_.bottom_bar_hidden = IsObjectHidden(bottom_bar_);
}

void WatchDisplay::RestoreAssistantLayerVisibility() {
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
