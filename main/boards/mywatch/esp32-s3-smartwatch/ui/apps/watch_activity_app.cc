#include "watch_activity_app.h"

#include <cstdio>

void WatchActivityApp::BuildContent(lv_obj_t* content) {
    AddLabel(content, "今日", 10, 0x929AA5);
    steps_ = AddLabel(content, "0 步", 55, 0x2FD17B);
    distance_ = AddLabel(content, "0.00 km", 125);
    calories_ = AddLabel(content, "0.0 kcal", 180);
    active_ = AddLabel(content, "活跃 0 分钟", 235);
    sensor_ = AddLabel(content, "运动传感器", 310, 0x929AA5);
}

void WatchActivityApp::Refresh() {
    const auto value = health_.GetSnapshot();
    char text[48];
    snprintf(text, sizeof(text), "%lu 步", static_cast<unsigned long>(value.steps));
    lv_label_set_text(steps_, text);
    snprintf(text, sizeof(text), "%lu.%02lu km",
             static_cast<unsigned long>(value.distance_m / 1000),
             static_cast<unsigned long>((value.distance_m % 1000) / 10));
    lv_label_set_text(distance_, text);
    snprintf(text, sizeof(text), "%lu.%lu kcal",
             static_cast<unsigned long>(value.calories_tenths / 10),
             static_cast<unsigned long>(value.calories_tenths % 10));
    lv_label_set_text(calories_, text);
    snprintf(text, sizeof(text), "活跃 %lu 分钟", static_cast<unsigned long>(value.active_minutes));
    lv_label_set_text(active_, text);
    lv_label_set_text(sensor_, value.sensor_available ? "运动传感器正常" : "运动传感器不可用");
}
