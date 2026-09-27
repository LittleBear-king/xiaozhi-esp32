#include "watch_model.h"

#include <algorithm>

void WatchModel::UpdateBattery(int percent, bool charging, bool discharging) {
    battery_percent_.store(std::clamp(percent, 0, 100), std::memory_order_relaxed);
    charging_.store(charging, std::memory_order_relaxed);
    discharging_.store(discharging, std::memory_order_relaxed);
}

void WatchModel::SetNetworkState(WatchNetworkState state) {
    network_state_.store(state, std::memory_order_relaxed);
}

WatchSnapshot WatchModel::GetSnapshot() const {
    return {
        .battery_percent = battery_percent_.load(std::memory_order_relaxed),
        .charging = charging_.load(std::memory_order_relaxed),
        .discharging = discharging_.load(std::memory_order_relaxed),
        .network_state = network_state_.load(std::memory_order_relaxed),
    };
}
