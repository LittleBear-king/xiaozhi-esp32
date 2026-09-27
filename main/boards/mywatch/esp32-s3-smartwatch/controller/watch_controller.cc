#include "watch_controller.h"

#include "application.h"
#include "board.h"

void WatchController::UpdateBattery(int percent, bool charging, bool discharging) {
    model_.UpdateBattery(percent, charging, discharging);
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

void WatchController::RequestTalk() {
    Application::GetInstance().Schedule([]() { Application::GetInstance().ToggleChatState(); });
}

bool WatchController::IsIdle() const {
    return Application::GetInstance().GetDeviceState() == kDeviceStateIdle;
}
