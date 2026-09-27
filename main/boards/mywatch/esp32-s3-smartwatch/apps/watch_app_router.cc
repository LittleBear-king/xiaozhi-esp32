#include "watch_app_router.h"

#include <esp_log.h>

#include <utility>

#define TAG "WatchAppRouter"

WatchAppRouter::WatchAppRouter(Callbacks callbacks) : callbacks_(std::move(callbacks)) {}

WatchAppRouter::~WatchAppRouter() {
    if (active_ != nullptr && visible_) {
        active_->OnPause();
    }
}

bool WatchAppRouter::Register(std::unique_ptr<WatchApp> app) {
    if (app == nullptr || app_count_ >= apps_.size() || Find(app->Id()) != nullptr) {
        return false;
    }
    apps_[app_count_++] = std::move(app);
    return true;
}

void WatchAppRouter::Attach(lv_obj_t* parent) { parent_ = parent; }

void WatchAppRouter::Show(WatchAppId id) {
    visible_ = true;
    history_size_ = 0;
    Activate(id, false);
}

void WatchAppRouter::Hide() {
    if (active_ != nullptr && visible_) {
        active_->OnPause();
    }
    visible_ = false;
    history_size_ = 0;
}

void WatchAppRouter::Tick() {
    if (active_ != nullptr && visible_) {
        active_->OnTick();
    }
}

void WatchAppRouter::NavigateTo(WatchAppId id) { Activate(id, true); }

void WatchAppRouter::NavigateBack() {
    if (history_size_ == 0) {
        CloseApps();
        return;
    }
    const WatchAppId previous = history_[--history_size_];
    Activate(previous, false);
}

void WatchAppRouter::CloseApps() {
    Hide();
    if (callbacks_.on_close) {
        callbacks_.on_close();
    }
}

void WatchAppRouter::OpenAssistant() {
    Hide();
    if (callbacks_.on_assistant) {
        callbacks_.on_assistant();
    }
}

WatchApp* WatchAppRouter::Find(WatchAppId id) const {
    for (size_t index = 0; index < app_count_; ++index) {
        if (apps_[index] != nullptr && apps_[index]->Id() == id) {
            return apps_[index].get();
        }
    }
    return nullptr;
}

bool WatchAppRouter::EnsureCreated(WatchApp& app) {
    if (parent_ == nullptr) {
        return false;
    }
    for (size_t index = 0; index < app_count_; ++index) {
        if (apps_[index].get() != &app) {
            continue;
        }
        if (!created_[index]) {
            app.Create(parent_, *this);
            created_[index] = true;
        }
        return true;
    }
    return false;
}

void WatchAppRouter::Activate(WatchAppId id, bool remember) {
    WatchApp* next = Find(id);
    if (next == nullptr || !EnsureCreated(*next)) {
        ESP_LOGW(TAG, "App %u is not registered", static_cast<unsigned>(id));
        return;
    }
    if (active_ == next && visible_) {
        next->OnResume();
        return;
    }
    if (active_ != nullptr) {
        if (remember && history_size_ < history_.size()) {
            history_[history_size_++] = active_->Id();
        }
        active_->OnPause();
    }
    active_ = next;
    visible_ = true;
    active_->OnResume();
}
