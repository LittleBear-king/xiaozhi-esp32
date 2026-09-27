#include "watch_activity_app.h"

#include <cstdio>

void WatchActivityApp::BuildContent(lv_obj_t* content) {
    AddLabel(content, "TODAY", 10, 0x929AA5);
    steps_ = AddLabel(content, "0 STEPS", 55, 0x2FD17B);
    distance_ = AddLabel(content, "0.00 KM", 125);
    calories_ = AddLabel(content, "0.0 KCAL", 180);
    active_ = AddLabel(content, "0 ACTIVE MIN", 235);
    sensor_ = AddLabel(content, "MOTION SENSOR", 310, 0x929AA5);
}

void WatchActivityApp::Refresh() {
    const auto value = health_.GetSnapshot();
    char text[48];
    snprintf(text, sizeof(text), "%lu STEPS", static_cast<unsigned long>(value.steps));
    lv_label_set_text(steps_, text);
    snprintf(text, sizeof(text), "%lu.%02lu KM",
             static_cast<unsigned long>(value.distance_m / 1000),
             static_cast<unsigned long>((value.distance_m % 1000) / 10));
    lv_label_set_text(distance_, text);
    snprintf(text, sizeof(text), "%lu.%lu KCAL",
             static_cast<unsigned long>(value.calories_tenths / 10),
             static_cast<unsigned long>(value.calories_tenths % 10));
    lv_label_set_text(calories_, text);
    snprintf(text, sizeof(text), "%lu ACTIVE MIN",
             static_cast<unsigned long>(value.active_minutes));
    lv_label_set_text(active_, text);
    lv_label_set_text(sensor_,
                      value.sensor_available ? "MOTION SENSOR READY" : "MOTION UNAVAILABLE");
}
