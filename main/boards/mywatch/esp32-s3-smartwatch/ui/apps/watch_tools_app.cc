#include "watch_tools_app.h"

#include <lvgl.h>

#include <cstdio>

void WatchToolsApp::BuildContent(lv_obj_t* content) {
    stopwatch_ = AddButton(content, "秒表  00:00.0", 0, 5, 230, 74, StopwatchCallback, this);
    stopwatch_ = lv_obj_get_child(stopwatch_, 0);
    AddButton(content, "复位", 240, 5, 110, 74, StopwatchResetCallback, this);
    rtc_ = AddLabel(content, "RTC  正常 · 已同步", 96, 0x929AA5);
    phone_status_ = AddLabel(content, "手机  未连接 · 0 条", 144, 0x929AA5);
    diagnostics_ = AddLabel(content, "启动 0 · 故障 0", 192, 0x929AA5);
    AddLabel(content, "WDT  开启", 240, 0x929AA5);
    AddLabel(content, "天气 / 音乐 / 找手机", 288, 0x8BA4FF);
    AddLabel(content, "配对手机后可用", 330, 0x929AA5);
}

void WatchToolsApp::OnTick() {
    if (stopwatch_running_)
        Refresh();
}

void WatchToolsApp::Refresh() {
    int64_t elapsed = stopwatch_elapsed_ms_;
    if (stopwatch_running_)
        elapsed += static_cast<int64_t>(lv_tick_get()) - stopwatch_started_ms_;
    char text[96];
    snprintf(text, sizeof(text), "秒表  %02lld:%02lld.%lld",
             static_cast<long long>((elapsed / 60000) % 100),
             static_cast<long long>((elapsed / 1000) % 60),
             static_cast<long long>((elapsed / 100) % 10));
    lv_label_set_text(stopwatch_, text);

    const auto clock = time_.GetSnapshot();
    snprintf(text, sizeof(text), "RTC  %s · %s", clock.rtc_available ? "正常" : "不可用",
             clock.system_time_valid ? "已同步" : "待同步");
    lv_label_set_text(rtc_, text);

    const auto phone = phone_.GetSnapshot();
    snprintf(text, sizeof(text), "手机  %s · %lu 条", phone.connected ? "已连接" : "未连接",
             static_cast<unsigned long>(phone.received_messages));
    lv_label_set_text(phone_status_, text);

    const auto diag = reliability_.GetSnapshot();
    snprintf(text, sizeof(text), "启动 %lu · 故障 %lu",
             static_cast<unsigned long>(diag.boot_count),
             static_cast<unsigned long>(diag.consecutive_faults));
    lv_label_set_text(diagnostics_, text);
}

void WatchToolsApp::StopwatchCallback(lv_event_t* event) {
    auto* app = static_cast<WatchToolsApp*>(lv_event_get_user_data(event));
    const int64_t now = lv_tick_get();
    if (app->stopwatch_running_) {
        app->stopwatch_elapsed_ms_ += now - app->stopwatch_started_ms_;
        app->stopwatch_running_ = false;
    } else {
        app->stopwatch_started_ms_ = now;
        app->stopwatch_running_ = true;
    }
    app->Refresh();
}

void WatchToolsApp::StopwatchResetCallback(lv_event_t* event) {
    auto* app = static_cast<WatchToolsApp*>(lv_event_get_user_data(event));
    app->stopwatch_running_ = false;
    app->stopwatch_started_ms_ = 0;
    app->stopwatch_elapsed_ms_ = 0;
    app->Refresh();
}
