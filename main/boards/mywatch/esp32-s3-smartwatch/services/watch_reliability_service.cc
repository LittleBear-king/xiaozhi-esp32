#include "watch_reliability_service.h"

#include "settings.h"

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_system.h>

#define TAG "WatchReliability"

namespace {
constexpr int64_t kStableBootPeriodUs = 60LL * 1000 * 1000;

bool IsFaultReset(esp_reset_reason_t reason) {
    return reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT || reason == ESP_RST_TASK_WDT ||
           reason == ESP_RST_WDT || reason == ESP_RST_BROWNOUT;
}
}  // namespace

WatchReliabilityService::WatchReliabilityService() {
    const auto reset_reason = esp_reset_reason();
    Settings stored("watch_diag", false);
    snapshot_.boot_count = static_cast<uint32_t>(stored.GetInt("boots", 0)) + 1;
    snapshot_.consecutive_faults = static_cast<uint32_t>(stored.GetInt("faults", 0));
    if (IsFaultReset(reset_reason))
        ++snapshot_.consecutive_faults;
    snapshot_.reset_reason = static_cast<int>(reset_reason);
#if CONFIG_ESP_TASK_WDT_EN
    snapshot_.task_watchdog_enabled = true;
#endif

    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;
    snapshot_.ota_pending_verification = running != nullptr &&
                                         esp_ota_get_state_partition(running, &state) == ESP_OK &&
                                         state == ESP_OTA_IMG_PENDING_VERIFY;

    {
        Settings settings("watch_diag", true);
        settings.SetInt("boots", static_cast<int32_t>(snapshot_.boot_count));
        settings.SetInt("faults", static_cast<int32_t>(snapshot_.consecutive_faults));
        settings.SetInt("reset", snapshot_.reset_reason);
    }

    const esp_timer_create_args_t args = {
        .callback = StableTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "watch_stable",
        .skip_unhandled_events = true,
    };
    if (esp_timer_create(&args, &stable_timer_) == ESP_OK) {
        esp_timer_start_once(stable_timer_, kStableBootPeriodUs);
    }
    ESP_LOGI(TAG, "Boot %lu, reset=%d, consecutive faults=%lu, rollback pending=%d",
             static_cast<unsigned long>(snapshot_.boot_count), snapshot_.reset_reason,
             static_cast<unsigned long>(snapshot_.consecutive_faults),
             snapshot_.ota_pending_verification);
}

WatchReliabilityService::~WatchReliabilityService() {
    if (stable_timer_ != nullptr) {
        esp_timer_stop(stable_timer_);
        esp_timer_delete(stable_timer_);
    }
}

WatchReliabilitySnapshot WatchReliabilityService::GetSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return snapshot_;
}

void WatchReliabilityService::StableTimerCallback(void* context) {
    static_cast<WatchReliabilityService*>(context)->MarkStable();
}

void WatchReliabilityService::MarkStable() {
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_.consecutive_faults = 0;
    Settings settings("watch_diag", true);
    settings.SetInt("faults", 0);
    ESP_LOGI(TAG, "Boot marked stable after 60 seconds");
}
