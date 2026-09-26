#include "watch_display.h"

#include "application.h"
#include "assets/lang_config.h"

#include <cstring>
#include <ctime>

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

WatchDisplay::WatchDisplay(esp_lcd_panel_io_handle_t io_handle,
                           esp_lcd_panel_handle_t panel_handle, int width, int height,
                           int offset_x, int offset_y, bool mirror_x, bool mirror_y, bool swap_xy)
    : SpiLcdDisplay(io_handle, panel_handle, width, height, offset_x, offset_y, mirror_x,
                    mirror_y, swap_xy) {}

WatchDisplay::~WatchDisplay() {
    DisplayLockGuard lock(this);
    if (clock_timer_ != nullptr) {
        lv_timer_delete(clock_timer_);
        clock_timer_ = nullptr;
    }
    if (watch_face_ != nullptr) {
        lv_obj_del(watch_face_);
        watch_face_ = nullptr;
    }
}

void WatchDisplay::RounderEventCallback(lv_event_t* event) {
    auto* area = static_cast<lv_area_t*>(lv_event_get_param(event));
    area->x1 = (area->x1 >> 1) << 1;
    area->y1 = (area->y1 >> 1) << 1;
    area->x2 = ((area->x2 >> 1) << 1) + 1;
    area->y2 = ((area->y2 >> 1) << 1) + 1;
}

void WatchDisplay::ClockTimerCallback(lv_timer_t* timer) {
    auto* display = static_cast<WatchDisplay*>(lv_timer_get_user_data(timer));
    display->UpdateClock();

    if (Application::GetInstance().GetDeviceState() == kDeviceStateIdle &&
        display->notification_label_ != nullptr && display->status_label_ != nullptr &&
        lv_obj_has_flag(display->notification_label_, LV_OBJ_FLAG_HIDDEN)) {
        const char* status = lv_label_get_text(display->status_label_);
        display->ApplyWatchFaceVisibility(display->IsIdleFaceStatus(status));
    }
}

void WatchDisplay::AssistantButtonCallback(lv_event_t* event) {
    auto* display = static_cast<WatchDisplay*>(lv_event_get_user_data(event));
    display->ApplyWatchFaceVisibility(false);
    Application::GetInstance().ToggleChatState();
}

void WatchDisplay::CreateWatchFace() {
    watch_face_ = lv_obj_create(container_);
    lv_obj_set_size(watch_face_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(watch_face_, 0, 0);
    lv_obj_set_style_radius(watch_face_, 0, 0);
    lv_obj_set_style_border_width(watch_face_, 0, 0);
    lv_obj_set_style_pad_all(watch_face_, 0, 0);
    lv_obj_set_style_bg_color(watch_face_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(watch_face_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(watch_face_, LV_SCROLLBAR_MODE_OFF);

    brand_label_ = lv_label_create(watch_face_);
    lv_label_set_text(brand_label_, "MYWATCH");
    lv_obj_set_style_text_font(brand_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(brand_label_, lv_color_hex(0x7F8CFF), 0);
    lv_obj_set_style_text_letter_space(brand_label_, 3, 0);
    lv_obj_align(brand_label_, LV_ALIGN_TOP_MID, 0, 62);

    time_label_ = lv_label_create(watch_face_);
    lv_label_set_text(time_label_, "--:--");
    lv_obj_set_style_text_font(time_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(time_label_, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(time_label_, 4, 0);
    lv_obj_align(time_label_, LV_ALIGN_CENTER, 0, -92);

    date_label_ = lv_label_create(watch_face_);
    lv_label_set_text(date_label_, "Waiting for time");
    lv_obj_set_style_text_font(date_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(date_label_, lv_color_hex(0x9AA0AA), 0);
    lv_obj_align(date_label_, LV_ALIGN_CENTER, 0, -43);

    assistant_button_ = lv_button_create(watch_face_);
    lv_obj_set_size(assistant_button_, 116, 116);
    lv_obj_set_style_radius(assistant_button_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(assistant_button_, lv_color_hex(0x315CFF), 0);
    lv_obj_set_style_bg_opa(assistant_button_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(assistant_button_, 2, 0);
    lv_obj_set_style_border_color(assistant_button_, lv_color_hex(0x8BA4FF), 0);
    lv_obj_set_style_shadow_width(assistant_button_, 18, 0);
    lv_obj_set_style_shadow_color(assistant_button_, lv_color_hex(0x2448CC), 0);
    lv_obj_set_style_shadow_opa(assistant_button_, LV_OPA_50, 0);
    lv_obj_align(assistant_button_, LV_ALIGN_CENTER, 0, 72);
    lv_obj_add_event_cb(assistant_button_, AssistantButtonCallback, LV_EVENT_CLICKED, this);

    assistant_label_ = lv_label_create(assistant_button_);
    lv_label_set_text(assistant_label_, "AI");
    lv_obj_set_style_text_font(assistant_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(assistant_label_, lv_color_white(), 0);
    lv_obj_center(assistant_label_);

    hint_label_ = lv_label_create(watch_face_);
    lv_label_set_text(hint_label_, "TAP TO TALK");
    lv_obj_set_style_text_font(hint_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(hint_label_, lv_color_hex(0x8A8F99), 0);
    lv_obj_set_style_text_letter_space(hint_label_, 2, 0);
    lv_obj_align(hint_label_, LV_ALIGN_BOTTOM_MID, 0, -54);

    lv_obj_add_flag(watch_face_, LV_OBJ_FLAG_HIDDEN);
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

    CreateWatchFace();
    UpdateClock();
    clock_timer_ = lv_timer_create(ClockTimerCallback, 1000, this);
}

void WatchDisplay::SetStatus(const char* status) {
    SpiLcdDisplay::SetStatus(status);
    UpdateWatchFaceVisibility(status);
}

void WatchDisplay::ShowNotification(const char* notification, int duration_ms) {
    SpiLcdDisplay::ShowNotification(notification, duration_ms);
    SetWatchFaceVisible(false);
}

void WatchDisplay::UpdateClock() {
    if (time_label_ == nullptr || date_label_ == nullptr) {
        return;
    }

    const time_t now = time(nullptr);
    struct tm time_info = {};
    localtime_r(&now, &time_info);

    if (time_info.tm_year < 2025 - 1900) {
        lv_label_set_text(time_label_, "--:--");
        lv_label_set_text(date_label_, "Waiting for time");
        return;
    }

    char time_text[8];
    char date_text[16];
    strftime(time_text, sizeof(time_text), "%H:%M", &time_info);
    strftime(date_text, sizeof(date_text), "%Y-%m-%d", &time_info);
    lv_label_set_text(time_label_, time_text);
    lv_label_set_text(date_label_, date_text);
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
    const bool show = Application::GetInstance().GetDeviceState() == kDeviceStateIdle &&
                      IsIdleFaceStatus(status);
    SetWatchFaceVisible(show);
}

void WatchDisplay::SetWatchFaceVisible(bool visible) {
    DisplayLockGuard lock(this);
    ApplyWatchFaceVisibility(visible);
}

void WatchDisplay::ApplyWatchFaceVisibility(bool visible) {
    if (watch_face_ == nullptr || visible == watch_face_visible_) {
        return;
    }

    watch_face_visible_ = visible;
    if (visible) {
        UpdateClock();
        status_bar_was_hidden_ = lv_obj_has_flag(status_bar_, LV_OBJ_FLAG_HIDDEN);
        emoji_box_was_hidden_ =
            emoji_box_ == nullptr || lv_obj_has_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
        preview_image_was_hidden_ =
            preview_image_ == nullptr || lv_obj_has_flag(preview_image_, LV_OBJ_FLAG_HIDDEN);
        bottom_bar_was_hidden_ =
            bottom_bar_ == nullptr || lv_obj_has_flag(bottom_bar_, LV_OBJ_FLAG_HIDDEN);

        lv_obj_remove_flag(watch_face_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(status_bar_, LV_OBJ_FLAG_HIDDEN);
        if (emoji_box_ != nullptr) {
            lv_obj_add_flag(emoji_box_, LV_OBJ_FLAG_HIDDEN);
        }
        if (preview_image_ != nullptr) {
            lv_obj_add_flag(preview_image_, LV_OBJ_FLAG_HIDDEN);
        }
        if (bottom_bar_ != nullptr) {
            lv_obj_add_flag(bottom_bar_, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_obj_add_flag(watch_face_, LV_OBJ_FLAG_HIDDEN);
        RestoreObjectVisibility(status_bar_, status_bar_was_hidden_);
        RestoreObjectVisibility(emoji_box_, emoji_box_was_hidden_);
        RestoreObjectVisibility(preview_image_, preview_image_was_hidden_);
        RestoreObjectVisibility(bottom_bar_, bottom_bar_was_hidden_);
    }
}

void WatchDisplay::RestoreObjectVisibility(lv_obj_t* object, bool was_hidden) {
    if (object == nullptr) {
        return;
    }
    if (was_hidden) {
        lv_obj_add_flag(object, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(object, LV_OBJ_FLAG_HIDDEN);
    }
}
