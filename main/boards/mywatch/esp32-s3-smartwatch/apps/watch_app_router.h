#ifndef _MYWATCH_WATCH_APP_ROUTER_H_
#define _MYWATCH_WATCH_APP_ROUTER_H_

#include "watch_app.h"

#include <array>
#include <cstddef>
#include <functional>
#include <memory>

class WatchAppRouter final : public WatchAppNavigator {
public:
    struct Callbacks {
        std::function<void()> on_close;
        std::function<void()> on_assistant;
    };

    explicit WatchAppRouter(Callbacks callbacks);
    ~WatchAppRouter();

    bool Register(std::unique_ptr<WatchApp> app);
    void Attach(lv_obj_t* parent);
    void Show(WatchAppId id = WatchAppId::kLauncher);
    void Hide();
    void Tick();
    bool IsVisible() const { return visible_; }

    void NavigateTo(WatchAppId id) override;
    void NavigateBack() override;
    void CloseApps() override;
    void OpenAssistant() override;

private:
    static constexpr size_t kMaximumApps = 8;
    static constexpr size_t kMaximumHistory = 6;

    WatchApp* Find(WatchAppId id) const;
    bool EnsureCreated(WatchApp& app);
    void Activate(WatchAppId id, bool remember);

    Callbacks callbacks_;
    lv_obj_t* parent_ = nullptr;
    std::array<std::unique_ptr<WatchApp>, kMaximumApps> apps_{};
    std::array<bool, kMaximumApps> created_{};
    size_t app_count_ = 0;
    std::array<WatchAppId, kMaximumHistory> history_{};
    size_t history_size_ = 0;
    WatchApp* active_ = nullptr;
    bool visible_ = false;
};

#endif  // _MYWATCH_WATCH_APP_ROUTER_H_
