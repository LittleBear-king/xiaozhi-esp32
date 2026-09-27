#include "watch_face.h"

#include "watch_ui_tokens.h"

#include <ctime>

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

WatchFace::WatchFace(lv_obj_t* parent, const Callbacks& callbacks) : callbacks_(callbacks) {
    Create(parent);
    RefreshClock(true);
    clock_timer_ = lv_timer_create(ClockTimerCallback, watch_ui::kClockRefreshPeriodMs, this);
}

WatchFace::~WatchFace() {
    if (clock_timer_ != nullptr) {
        lv_timer_delete(clock_timer_);
        clock_timer_ = nullptr;
    }
    if (root_ != nullptr) {
        lv_obj_delete(root_);
        root_ = nullptr;
    }
}

void WatchFace::ClockTimerCallback(lv_timer_t* timer) {
    auto* face = static_cast<WatchFace*>(lv_timer_get_user_data(timer));
    face->RefreshClock();
    if (face->callbacks_.on_tick != nullptr) {
        face->callbacks_.on_tick(face->callbacks_.context);
    }
}

void WatchFace::TalkButtonCallback(lv_event_t* event) {
    auto* face = static_cast<WatchFace*>(lv_event_get_user_data(event));
    if (face->callbacks_.on_talk != nullptr) {
        face->callbacks_.on_talk(face->callbacks_.context);
    }
}

void WatchFace::Create(lv_obj_t* parent) {
    root_ = lv_obj_create(parent);
    lv_obj_set_size(root_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(root_, 0, 0);
    lv_obj_set_style_radius(root_, 0, 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_hex(watch_ui::kBackgroundColor), 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(root_, LV_SCROLLBAR_MODE_OFF);

    auto* brand_label = lv_label_create(root_);
    lv_label_set_text(brand_label, watch_ui::kBrandText);
    lv_obj_set_style_text_font(brand_label, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(brand_label, lv_color_hex(watch_ui::kBrandColor), 0);
    lv_obj_set_style_text_letter_space(brand_label, 3, 0);
    lv_obj_align(brand_label, LV_ALIGN_TOP_MID, 0, watch_ui::kBrandTopOffset);

    time_label_ = lv_label_create(root_);
    lv_label_set_text(time_label_, "--:--");
    lv_obj_set_style_text_font(time_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(time_label_, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(time_label_, 4, 0);
    lv_obj_align(time_label_, LV_ALIGN_CENTER, 0, watch_ui::kTimeCenterOffsetY);

    date_label_ = lv_label_create(root_);
    lv_label_set_text(date_label_, watch_ui::kWaitingForTimeText);
    lv_obj_set_style_text_font(date_label_, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(date_label_, lv_color_hex(watch_ui::kSecondaryTextColor), 0);
    lv_obj_align(date_label_, LV_ALIGN_CENTER, 0, watch_ui::kDateCenterOffsetY);

    auto* talk_button = lv_button_create(root_);
    lv_obj_set_size(talk_button, watch_ui::kTalkButtonSize, watch_ui::kTalkButtonSize);
    lv_obj_set_style_radius(talk_button, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(talk_button, lv_color_hex(watch_ui::kPrimaryColor), 0);
    lv_obj_set_style_bg_opa(talk_button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(talk_button, watch_ui::kTalkButtonBorderWidth, 0);
    lv_obj_set_style_border_color(talk_button, lv_color_hex(watch_ui::kPrimaryBorderColor), 0);
    lv_obj_set_style_shadow_width(talk_button, watch_ui::kTalkButtonShadowWidth, 0);
    lv_obj_set_style_shadow_color(talk_button, lv_color_hex(watch_ui::kPrimaryShadowColor), 0);
    lv_obj_set_style_shadow_opa(talk_button, LV_OPA_50, 0);
    lv_obj_align(talk_button, LV_ALIGN_CENTER, 0, watch_ui::kTalkButtonCenterOffsetY);
    lv_obj_add_event_cb(talk_button, TalkButtonCallback, LV_EVENT_CLICKED, this);

    auto* talk_label = lv_label_create(talk_button);
    lv_label_set_text(talk_label, watch_ui::kTalkButtonText);
    lv_obj_set_style_text_font(talk_label, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(talk_label, lv_color_white(), 0);
    lv_obj_center(talk_label);

    auto* hint_label = lv_label_create(root_);
    lv_label_set_text(hint_label, watch_ui::kTalkHintText);
    lv_obj_set_style_text_font(hint_label, &BUILTIN_TEXT_FONT, 0);
    lv_obj_set_style_text_color(hint_label, lv_color_hex(watch_ui::kHintTextColor), 0);
    lv_obj_set_style_text_letter_space(hint_label, 2, 0);
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, watch_ui::kHintBottomOffset);

    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void WatchFace::SetVisible(bool visible) {
    if (root_ == nullptr || visible == visible_) {
        return;
    }
    visible_ = visible;
    if (visible) {
        RefreshClock(true);
        lv_obj_remove_flag(root_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
    }
}

void WatchFace::RefreshClock(bool force) {
    if (time_label_ == nullptr || date_label_ == nullptr) {
        return;
    }

    const time_t now = time(nullptr);
    struct tm time_info = {};
    localtime_r(&now, &time_info);

    if (time_info.tm_year < watch_ui::kMinimumValidYear - 1900) {
        if (force || last_minute_ != -2) {
            lv_label_set_text(time_label_, "--:--");
            lv_label_set_text(date_label_, watch_ui::kWaitingForTimeText);
            last_minute_ = -2;
            last_day_ = -1;
        }
        return;
    }

    if (force || time_info.tm_min != last_minute_) {
        char time_text[8];
        strftime(time_text, sizeof(time_text), "%H:%M", &time_info);
        lv_label_set_text(time_label_, time_text);
        last_minute_ = time_info.tm_min;
    }

    if (force || time_info.tm_yday != last_day_) {
        char date_text[16];
        strftime(date_text, sizeof(date_text), "%Y-%m-%d", &time_info);
        lv_label_set_text(date_label_, date_text);
        last_day_ = time_info.tm_yday;
    }
}
