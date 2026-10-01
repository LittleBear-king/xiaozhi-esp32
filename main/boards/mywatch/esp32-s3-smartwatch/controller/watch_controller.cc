#include "watch_controller.h"

#include "application.h"
#include "board.h"
#include "services/watch_power_policy.h"

void WatchController::AttachPowerPolicy(WatchPowerPolicy& power_policy) {
    power_policy_.store(&power_policy);
}

void WatchController::UpdateBattery(int percent, bool charging, bool discharging) {
    model_.UpdateBattery(percent, charging, discharging);

    const int next_state = discharging ? 1 : 0;
    if (discharging_state_.exchange(next_state) == next_state || power_policy_.load() == nullptr) {
        return;
    }

    auto* power_policy = power_policy_.load();
    Application::GetInstance().Schedule(
        [power_policy, discharging]() { power_policy->UpdatePowerSource(discharging); });
}

void WatchController::HandleNetworkEvent(NetworkEvent event) {
    switch (event) {
        case NetworkEvent::Connected:
            model_.SetNetworkState(WatchNetworkState::kOnline);
            break;
        case NetworkEvent::Scanning:
        case NetworkEvent::Connecting:
        case NetworkEvent::WifiConfigModeExit:
            model_.SetNetworkState(WatchNetworkState::kConnecting);
            break;
        case NetworkEvent::Disconnected:
        case NetworkEvent::WifiConfigModeEnter:
        default:
            model_.SetNetworkState(WatchNetworkState::kOffline);
            break;
    }
}

void WatchController::NotifyUserActivity() {
    auto* power_policy = power_policy_.load();
    if (power_policy == nullptr) {
        return;
    }
    Application::GetInstance().Schedule([power_policy]() { power_policy->WakeDisplay(); });
}

void WatchController::RequestTalk() {
    auto* power_policy = power_policy_.load();
    Application::GetInstance().Schedule([power_policy]() {
        if (power_policy != nullptr) {
            power_policy->WakeDisplay();
        }
        Application::GetInstance().RequestChat();
    });
}

bool WatchController::IsIdle() const {
    return Application::GetInstance().GetDeviceState() == kDeviceStateIdle;
}
