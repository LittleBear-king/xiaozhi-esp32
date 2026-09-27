#ifndef _MYWATCH_WATCH_CONTROLLER_H_
#define _MYWATCH_WATCH_CONTROLLER_H_

#include "model/watch_model.h"

#include <atomic>

enum class NetworkEvent;
class WatchPowerPolicy;

class WatchController final {
public:
    explicit WatchController(WatchModel& model) : model_(model) {}

    void AttachPowerPolicy(WatchPowerPolicy& power_policy);
    void UpdateBattery(int percent, bool charging, bool discharging);
    void HandleNetworkEvent(NetworkEvent event);
    void NotifyUserActivity();
    void RequestTalk();
    bool IsIdle() const;

private:
    WatchModel& model_;
    std::atomic<WatchPowerPolicy*> power_policy_{nullptr};
    std::atomic<int> discharging_state_{-1};
};

#endif  // _MYWATCH_WATCH_CONTROLLER_H_
