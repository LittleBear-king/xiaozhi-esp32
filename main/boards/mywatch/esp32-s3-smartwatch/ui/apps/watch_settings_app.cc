#include "watch_settings_app.h"

#include <cstdio>

namespace {
lv_obj_t* ButtonLabel(lv_obj_t* button) { return lv_obj_get_child(button, 0); }
}  // namespace

void WatchSettingsApp::BuildContent(lv_obj_t* content) {
    auto* button = AddButton(content, "", 0, 10, 350, 68, RaiseCallback, this);
    labels_[0] = ButtonLabel(button);
    button = AddButton(content, "", 0, 88, 350, 68, DndCallback, this);
    labels_[1] = ButtonLabel(button);
    button = AddButton(content, "", 0, 166, 350, 68, ClockCallback, this);
    labels_[2] = ButtonLabel(button);
    button = AddButton(content, "", 0, 244, 350, 68, BrightnessCallback, this);
    labels_[3] = ButtonLabel(button);
}

void WatchSettingsApp::Refresh() {
    const auto value = settings_.GetSnapshot();
    lv_label_set_text(labels_[0],
                      value.raise_to_wake ? "RAISE TO WAKE     ON" : "RAISE TO WAKE     OFF");
    lv_label_set_text(labels_[1],
                      value.do_not_disturb ? "DO NOT DISTURB    ON" : "DO NOT DISTURB    OFF");
    lv_label_set_text(labels_[2],
                      value.use_24_hour ? "CLOCK FORMAT      24H" : "CLOCK FORMAT      12H");
    char text[40];
    snprintf(text, sizeof(text), "BRIGHTNESS        %u%%", value.brightness);
    lv_label_set_text(labels_[3], text);
}

void WatchSettingsApp::RaiseCallback(lv_event_t* event) {
    auto* app = static_cast<WatchSettingsApp*>(lv_event_get_user_data(event));
    app->settings_.SetRaiseToWake(!app->settings_.GetSnapshot().raise_to_wake);
    app->Refresh();
}
void WatchSettingsApp::DndCallback(lv_event_t* event) {
    auto* app = static_cast<WatchSettingsApp*>(lv_event_get_user_data(event));
    app->settings_.SetDoNotDisturb(!app->settings_.GetSnapshot().do_not_disturb);
    app->Refresh();
}
void WatchSettingsApp::ClockCallback(lv_event_t* event) {
    auto* app = static_cast<WatchSettingsApp*>(lv_event_get_user_data(event));
    app->settings_.SetUse24Hour(!app->settings_.GetSnapshot().use_24_hour);
    app->Refresh();
}
void WatchSettingsApp::BrightnessCallback(lv_event_t* event) {
    auto* app = static_cast<WatchSettingsApp*>(lv_event_get_user_data(event));
    const uint8_t current = app->settings_.GetSnapshot().brightness;
    app->settings_.SetBrightness(current >= 100 ? 25 : current + 25);
    app->Refresh();
}
