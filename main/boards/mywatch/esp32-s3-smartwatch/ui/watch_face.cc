#include "watch_face.h"

#include "watch_ui_tokens.h"

#include <cstdio>
#include <ctime>

namespace {
constexpr const char* kWeekdays[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
}

WatchFace::WatchFace(lv_obj_t* parent, WatchModel& model, WatchSettingsService& settings,
                     const Callbacks& callbacks)
    : model_(model),
      settings_(settings),
      callbacks_(callbacks) {
    Create(parent);
    Refresh(true);
    refresh_timer_ = lv_timer_create(RefreshTimerCallback, watch_ui::kRefreshPeriodMs, this);
}

WatchFace::~WatchFace() {
    if (refresh_timer_ != nullptr) {
        lv_timer_delete(refresh_timer_);
        refresh_timer_ = nullptr;
    }
    if (root_ != nullptr) {
        lv_obj_delete(root_);
        root_ = nullptr;
    }
}

void WatchFace::RefreshTimerCallback(lv_timer_t* timer) {
    auto* face = static_cast<WatchFace*>(lv_timer_get_user_data(timer));
    face->Refresh();
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

void WatchFace::AppsButtonCallback(lv_event_t* event) {
    auto* face = static_cast<WatchFace*>(lv_event_get_user_data(event));
    if (face->callbacks_.on_apps != nullptr) {
        face->callbacks_.on_apps(face->callbacks_.context);
    }
}

void WatchFace::Create(lv_obj_t* parent) {
    root_ = lv_obj_create(parent);
    lv_obj_set_size(root_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(root_, 0, 0);
    lv_obj_set_style_radius(root_, watch_ui::kScreenCornerRadius, 0);
    lv_obj_set_style_clip_corner(root_, true, 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_hex(watch_ui::kBackgroundColor), 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(root_, LV_SCROLLBAR_MODE_OFF);

    network_label_ = lv_label_create(root_);
    lv_label_set_text(network_label_, watch_ui::kNetworkOfflineText);
    lv_obj_set_style_text_color(network_label_, lv_color_hex(watch_ui::kDangerColor), 0);
    lv_obj_align(network_label_, LV_ALIGN_TOP_LEFT, watch_ui::kStatusSideOffset,
                 watch_ui::kStatusTopOffset);

    battery_label_ = lv_label_create(root_);
    lv_label_set_text(battery_label_, "--%");
    lv_obj_set_style_text_color(battery_label_, lv_color_hex(watch_ui::kSecondaryTextColor), 0);
    lv_obj_align(battery_label_, LV_ALIGN_TOP_RIGHT, -watch_ui::kStatusSideOffset,
                 watch_ui::kStatusTopOffset);

    auto* brand_label = lv_label_create(root_);
    lv_label_set_text(brand_label, watch_ui::kBrandText);
    lv_obj_set_style_text_color(brand_label, lv_color_hex(watch_ui::kBrandColor), 0);
    lv_obj_set_style_text_letter_space(brand_label, 3, 0);
    lv_obj_align(brand_label, LV_ALIGN_TOP_MID, 0, watch_ui::kBrandTopOffset);

    time_label_ = lv_label_create(root_);
    lv_label_set_text(time_label_, "--:--");
    lv_obj_set_style_text_color(time_label_, lv_color_white(), 0);
    lv_obj_set_style_text_letter_space(time_label_, 4, 0);
    lv_obj_align(time_label_, LV_ALIGN_CENTER, 0, watch_ui::kTimeCenterOffsetY);

    date_label_ = lv_label_create(root_);
    lv_label_set_text(date_label_, watch_ui::kWaitingForTimeText);
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
    lv_obj_set_style_text_color(talk_label, lv_color_white(), 0);
    lv_obj_center(talk_label);

    auto* hint_label = lv_label_create(root_);
    lv_label_set_text(hint_label, watch_ui::kTalkHintText);
    lv_obj_set_style_text_color(hint_label, lv_color_hex(watch_ui::kHintTextColor), 0);
    lv_obj_set_style_text_letter_space(hint_label, 2, 0);
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -82);

    auto* apps_button = lv_button_create(root_);
    lv_obj_set_size(apps_button, 120, 44);
    lv_obj_align(apps_button, LV_ALIGN_BOTTOM_MID, 0, -27);
    lv_obj_set_style_radius(apps_button, 16, 0);
    lv_obj_set_style_bg_color(apps_button, lv_color_hex(0x20242A), 0);
    lv_obj_set_style_border_width(apps_button, 1, 0);
    lv_obj_set_style_border_color(apps_button, lv_color_hex(0x343A41), 0);
    lv_obj_add_event_cb(apps_button, AppsButtonCallback, LV_EVENT_CLICKED, this);
    auto* apps_label = lv_label_create(apps_button);
    lv_label_set_text(apps_label, "应用");
    lv_obj_set_style_text_color(apps_label, lv_color_white(), 0);
    lv_obj_center(apps_label);

    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void WatchFace::SetVisible(bool visible) {
    if (root_ == nullptr || visible == visible_) {
        return;
    }
    visible_ = visible;
    if (visible) {
        Refresh(true);
        lv_obj_move_foreground(root_);
        lv_obj_remove_flag(root_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
    }
}

void WatchFace::Refresh(bool force) {
    RefreshTime(force);
    RefreshStatus(force);
}

void WatchFace::RefreshTime(bool force) {
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
        char time_text[12];
        strftime(time_text, sizeof(time_text),
                 settings_.GetSnapshot().use_24_hour ? "%H:%M" : "%I:%M", &time_info);
        lv_label_set_text(time_label_, time_text);
        last_minute_ = time_info.tm_min;
    }

    if (force || time_info.tm_yday != last_day_) {
        char date_text[48];
        snprintf(date_text, sizeof(date_text), "%04d年%02d月%02d日  %s", time_info.tm_year + 1900,
                 time_info.tm_mon + 1, time_info.tm_mday, kWeekdays[time_info.tm_wday]);
        lv_label_set_text(date_label_, date_text);
        last_day_ = time_info.tm_yday;
    }
}

void WatchFace::RefreshStatus(bool force) {
    if (network_label_ == nullptr || battery_label_ == nullptr) {
        return;
    }

    const WatchSnapshot snapshot = model_.GetSnapshot();
    if (force || !has_status_snapshot_ || snapshot.network_state != last_network_state_) {
        const char* text = watch_ui::kNetworkOfflineText;
        uint32_t color = watch_ui::kDangerColor;
        if (snapshot.network_state == WatchNetworkState::kOnline) {
            text = watch_ui::kNetworkOnlineText;
            color = watch_ui::kSuccessColor;
        } else if (snapshot.network_state == WatchNetworkState::kConnecting) {
            text = watch_ui::kNetworkConnectingText;
            color = watch_ui::kWarningColor;
        }
        lv_label_set_text(network_label_, text);
        lv_obj_set_style_text_color(network_label_, lv_color_hex(color), 0);
        last_network_state_ = snapshot.network_state;
    }

    if (force || !has_status_snapshot_ || snapshot.battery_percent != last_battery_percent_ ||
        snapshot.charging != last_charging_) {
        char battery_text[16];
        if (snapshot.battery_percent < 0) {
            snprintf(battery_text, sizeof(battery_text), "--%%");
        } else if (snapshot.charging) {
            snprintf(battery_text, sizeof(battery_text), "%d%% +", snapshot.battery_percent);
        } else {
            snprintf(battery_text, sizeof(battery_text), "%d%%", snapshot.battery_percent);
        }

        uint32_t color = watch_ui::kSecondaryTextColor;
        if (snapshot.charging) {
            color = watch_ui::kSuccessColor;
        } else if (snapshot.battery_percent >= 0 &&
                   snapshot.battery_percent <= watch_ui::kLowBatteryThreshold) {
            color = watch_ui::kDangerColor;
        }
        lv_label_set_text(battery_label_, battery_text);
        lv_obj_set_style_text_color(battery_label_, lv_color_hex(color), 0);
        last_battery_percent_ = snapshot.battery_percent;
        last_charging_ = snapshot.charging;
    }

    has_status_snapshot_ = true;
}
