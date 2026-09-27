#ifndef _MYWATCH_WATCH_MODEL_H_
#define _MYWATCH_WATCH_MODEL_H_

#include <atomic>
#include <cstdint>

enum class WatchNetworkState : uint8_t {
    kOffline,
    kConnecting,
    kOnline,
};

struct WatchSnapshot {
    int battery_percent = -1;
    bool charging = false;
    bool discharging = false;
    WatchNetworkState network_state = WatchNetworkState::kOffline;
};

class WatchModel final {
public:
    void UpdateBattery(int percent, bool charging, bool discharging);
    void SetNetworkState(WatchNetworkState state);
    WatchSnapshot GetSnapshot() const;

private:
    std::atomic<int> battery_percent_{-1};
    std::atomic<bool> charging_{false};
    std::atomic<bool> discharging_{false};
    std::atomic<WatchNetworkState> network_state_{WatchNetworkState::kOffline};
};

#endif  // _MYWATCH_WATCH_MODEL_H_
