#ifndef _MYWATCH_WATCH_ACTIVITY_APP_H_
#define _MYWATCH_WATCH_ACTIVITY_APP_H_

#include "services/watch_health_service.h"
#include "watch_app_base.h"

class WatchActivityApp final : public WatchAppBase {
public:
    explicit WatchActivityApp(WatchHealthService& health) : WatchAppBase("运动"), health_(health) {}
    WatchAppId Id() const override { return WatchAppId::kActivity; }
    void OnTick() override { Refresh(); }

protected:
    void BuildContent(lv_obj_t* content) override;
    void Refresh() override;

private:
    WatchHealthService& health_;
    lv_obj_t* steps_ = nullptr;
    lv_obj_t* distance_ = nullptr;
    lv_obj_t* calories_ = nullptr;
    lv_obj_t* active_ = nullptr;
    lv_obj_t* sensor_ = nullptr;
};

#endif  // _MYWATCH_WATCH_ACTIVITY_APP_H_
