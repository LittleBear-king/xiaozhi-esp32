#ifndef _MYWATCH_WATCH_NOTIFICATIONS_APP_H_
#define _MYWATCH_WATCH_NOTIFICATIONS_APP_H_

#include "services/watch_notification_service.h"
#include "watch_app_base.h"

#include <array>

class WatchNotificationsApp final : public WatchAppBase {
public:
    explicit WatchNotificationsApp(WatchNotificationService& notifications)
        : WatchAppBase("NOTIFICATIONS"), notifications_(notifications) {}
    WatchAppId Id() const override { return WatchAppId::kNotifications; }
    void OnResume() override;
    void OnTick() override { Refresh(); }

protected:
    void BuildContent(lv_obj_t* content) override;
    void Refresh() override;

private:
    static void ClearCallback(lv_event_t* event);

    WatchNotificationService& notifications_;
    std::array<lv_obj_t*, 3> rows_{};
    lv_obj_t* empty_ = nullptr;
};

#endif  // _MYWATCH_WATCH_NOTIFICATIONS_APP_H_
