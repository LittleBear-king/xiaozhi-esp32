#ifndef _MYWATCH_WATCH_CONTROLLER_H_
#define _MYWATCH_WATCH_CONTROLLER_H_

#include "model/watch_model.h"

enum class NetworkEvent;

class WatchController final {
public:
    explicit WatchController(WatchModel& model) : model_(model) {}

    void UpdateBattery(int percent, bool charging, bool discharging);
    void HandleNetworkEvent(NetworkEvent event);
    void RequestTalk();
    bool IsIdle() const;

private:
    WatchModel& model_;
};

#endif  // _MYWATCH_WATCH_CONTROLLER_H_
