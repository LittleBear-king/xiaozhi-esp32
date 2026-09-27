#include "watch_tools_app.h"

#include <lvgl.h>

#include <cstdio>

void WatchToolsApp::BuildContent(lv_obj_t* content) {
    stopwatch_ = AddButton(content, "STOPWATCH  00:00.0", 0, 5, 350, 74, StopwatchCallback, this);
    stopwatch_ = lv_obj_get_child(stopwatch_, 0);
    rtc_ = AddLabel(content, "RTC", 100, 0x929AA5);
    phone_status_ = AddLabel(content, "PHONE", 155, 0x929AA5);
    diagnostics_ = AddLabel(content, "SYSTEM", 210, 0x929AA5);
    AddLabel(content, "WEATHER / MUSIC / FIND PHONE", 285, 0x8BA4FF);
    AddLabel(content, "AVAILABLE AFTER PHONE PAIRING", 325, 0x929AA5);
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
    snprintf(text, sizeof(text), "STOPWATCH  %02lld:%02lld.%lld",
             static_cast<long long>((elapsed / 60000) % 100),
             static_cast<long long>((elapsed / 1000) % 60),
             static_cast<long long>((elapsed / 100) % 10));
    lv_label_set_text(stopwatch_, text);

    const auto clock = time_.GetSnapshot();
    snprintf(text, sizeof(text), "RTC %s / TIME %s", clock.rtc_available ? "READY" : "MISSING",
             clock.system_time_valid ? "SYNCED" : "WAITING");
    lv_label_set_text(rtc_, text);

    const auto phone = phone_.GetSnapshot();
    snprintf(text, sizeof(text), "PHONE %s / RX %lu", phone.connected ? "CONNECTED" : "OFFLINE",
             static_cast<unsigned long>(phone.received_messages));
    lv_label_set_text(phone_status_, text);

    const auto diag = reliability_.GetSnapshot();
    snprintf(text, sizeof(text), "BOOT %lu / FAULTS %lu / WDT %s",
             static_cast<unsigned long>(diag.boot_count),
             static_cast<unsigned long>(diag.consecutive_faults),
             diag.task_watchdog_enabled ? "ON" : "OFF");
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
