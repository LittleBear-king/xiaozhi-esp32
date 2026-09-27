#ifndef _MYWATCH_WATCH_TOOLS_APP_H_
#define _MYWATCH_WATCH_TOOLS_APP_H_

#include "services/watch_phone_service.h"
#include "services/watch_reliability_service.h"
#include "services/watch_time_service.h"
#include "watch_app_base.h"

#include <cstdint>

class WatchToolsApp final : public WatchAppBase {
public:
    WatchToolsApp(WatchTimeService& time, WatchPhoneService& phone,
                  WatchReliabilityService& reliability)
        : WatchAppBase("工具"), time_(time), phone_(phone), reliability_(reliability) {}
    WatchAppId Id() const override { return WatchAppId::kTools; }
    void OnTick() override;

protected:
    void BuildContent(lv_obj_t* content) override;
    void Refresh() override;

private:
    static void StopwatchCallback(lv_event_t* event);

    WatchTimeService& time_;
    WatchPhoneService& phone_;
    WatchReliabilityService& reliability_;
    lv_obj_t* stopwatch_ = nullptr;
    lv_obj_t* rtc_ = nullptr;
    lv_obj_t* phone_status_ = nullptr;
    lv_obj_t* diagnostics_ = nullptr;
    bool stopwatch_running_ = false;
    int64_t stopwatch_started_ms_ = 0;
    int64_t stopwatch_elapsed_ms_ = 0;
};

#endif  // _MYWATCH_WATCH_TOOLS_APP_H_
