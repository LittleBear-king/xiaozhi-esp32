#ifndef _MYWATCH_WATCH_SETTINGS_APP_H_
#define _MYWATCH_WATCH_SETTINGS_APP_H_

#include "services/watch_settings_service.h"
#include "watch_app_base.h"

#include <array>

class WatchSettingsApp final : public WatchAppBase {
public:
    explicit WatchSettingsApp(WatchSettingsService& settings)
        : WatchAppBase("SETTINGS"), settings_(settings) {}
    WatchAppId Id() const override { return WatchAppId::kSettings; }

protected:
    void BuildContent(lv_obj_t* content) override;
    void Refresh() override;

private:
    static void RaiseCallback(lv_event_t* event);
    static void DndCallback(lv_event_t* event);
    static void ClockCallback(lv_event_t* event);
    static void BrightnessCallback(lv_event_t* event);

    WatchSettingsService& settings_;
    std::array<lv_obj_t*, 4> labels_{};
};

#endif  // _MYWATCH_WATCH_SETTINGS_APP_H_
