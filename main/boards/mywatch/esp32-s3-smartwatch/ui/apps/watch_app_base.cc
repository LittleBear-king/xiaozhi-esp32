#include "watch_app_base.h"

#include "ui/watch_ui_tokens.h"

WatchAppBase::~WatchAppBase() {
    if (root_ != nullptr) {
        lv_obj_delete(root_);
    }
}

void WatchAppBase::Create(lv_obj_t* parent, WatchAppNavigator& navigator) {
    navigator_ = &navigator;
    root_ = lv_obj_create(parent);
    lv_obj_set_size(root_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_pos(root_, 0, 0);
    lv_obj_set_style_radius(root_, watch_ui::kScreenCornerRadius, 0);
    lv_obj_set_style_clip_corner(root_, true, 0);
    lv_obj_set_style_border_width(root_, 0, 0);
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_hex(watch_ui::kBackgroundColor), 0);
    lv_obj_set_scrollbar_mode(root_, LV_SCROLLBAR_MODE_OFF);

    auto* back = lv_button_create(root_);
    lv_obj_set_size(back, 54, 42);
    lv_obj_align(back, LV_ALIGN_TOP_LEFT, 40, 35);
    lv_obj_set_style_radius(back, 16, 0);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x20242A), 0);
    lv_obj_set_style_border_width(back, 0, 0);
    lv_obj_add_event_cb(back, BackButtonCallback, LV_EVENT_CLICKED, this);
    auto* back_text = lv_label_create(back);
    lv_label_set_text(back_text, "<");
    lv_obj_set_style_text_color(back_text, lv_color_white(), 0);
    lv_obj_center(back_text);

    auto* title = lv_label_create(root_);
    lv_label_set_text(title, title_);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 43);

    content_ = lv_obj_create(root_);
    lv_obj_set_size(content_, 350, 380);
    lv_obj_align(content_, LV_ALIGN_BOTTOM_MID, 0, -32);
    lv_obj_set_style_bg_opa(content_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content_, 0, 0);
    lv_obj_set_style_pad_all(content_, 0, 0);
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);
    BuildContent(content_);
    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void WatchAppBase::OnResume() {
    Refresh();
    lv_obj_move_foreground(root_);
    lv_obj_remove_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void WatchAppBase::OnPause() { lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); }

lv_obj_t* WatchAppBase::AddLabel(lv_obj_t* parent, const char* text, int y, uint32_t color) {
    auto* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_height(label, 42);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y);
    return label;
}

lv_obj_t* WatchAppBase::AddButton(lv_obj_t* parent, const char* text, int x, int y, int width,
                                  int height, lv_event_cb_t callback, void* user_data) {
    auto* button = lv_button_create(parent);
    lv_obj_set_size(button, width, height);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_style_radius(button, 18, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x20242A), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(0x343A41), 0);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, user_data);
    auto* label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
    return button;
}

void WatchAppBase::BackButtonCallback(lv_event_t* event) {
    auto* app = static_cast<WatchAppBase*>(lv_event_get_user_data(event));
    app->Navigator().NavigateBack();
}
