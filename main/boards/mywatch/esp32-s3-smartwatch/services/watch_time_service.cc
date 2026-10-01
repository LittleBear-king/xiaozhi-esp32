#include "watch_time_service.h"

#include <esp_log.h>

#include <sys/time.h>

#define TAG "WatchTimeService"

namespace {
constexpr uint32_t kTaskStackSize = 3072;
constexpr UBaseType_t kTaskPriority = 1;
constexpr int kPollPeriodMs = 30000;
}  // namespace

WatchTimeService::~WatchTimeService() { Stop(); }

bool WatchTimeService::Start() {
    if (running_.exchange(true))
        return true;
    rtc_available_.store(rtc_.Initialize());
    TaskHandle_t task = nullptr;
    if (xTaskCreate(TaskEntry, "watch_time", kTaskStackSize, this, kTaskPriority, &task) !=
        pdPASS) {
        running_.store(false);
        return false;
    }
    task_.store(task);
    return true;
}

void WatchTimeService::Stop() {
    running_.store(false);
    TaskHandle_t task = task_.load();
    if (task == nullptr)
        return;
    xTaskNotifyGive(task);
    while (task_.load() != nullptr)
        vTaskDelay(1);
}

WatchTimeSnapshot WatchTimeService::GetSnapshot() const {
    return {
        .rtc_available = rtc_available_.load(),
        .system_time_valid = system_time_valid_.load(),
        .rtc_synchronized = rtc_synchronized_.load(),
    };
}

void WatchTimeService::SetAlarmProvider(std::function<bool(int, int)> provider,
                                        std::function<void()> callback) {
    alarm_provider_ = std::move(provider);
    alarm_callback_ = std::move(callback);
}

void WatchTimeService::TaskEntry(void* context) {
    static_cast<WatchTimeService*>(context)->TaskLoop();
}

void WatchTimeService::TaskLoop() {
    bool restored_from_rtc = false;
    while (running_.load()) {
        const time_t now = time(nullptr);
        const bool valid = IsTimeValid(now);
        system_time_valid_.store(valid);

        if (valid && alarm_provider_ && alarm_callback_) {
            struct tm local = {};
            localtime_r(&now, &local);
            const int minute_key = local.tm_yday * 1440 + local.tm_hour * 60 + local.tm_min;
            if (minute_key != last_alarm_minute_) {
                last_alarm_minute_ = minute_key;
                if (alarm_provider_(local.tm_hour, local.tm_min)) {
                    alarm_callback_();
                }
            }
        }

        if (!valid && rtc_available_.load() && !restored_from_rtc) {
            struct tm local = {};
            if (rtc_.Read(local)) {
                const time_t restored = mktime(&local);
                timeval value = {.tv_sec = restored, .tv_usec = 0};
                settimeofday(&value, nullptr);
                restored_from_rtc = true;
                system_time_valid_.store(IsTimeValid(restored));
                ESP_LOGI(TAG, "System clock restored from RTC");
            }
        } else if (valid && rtc_available_.load()) {
            struct tm local = {};
            localtime_r(&now, &local);
            if (rtc_.Write(local)) {
                rtc_synchronized_.store(true);
            }
        }
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(kPollPeriodMs));
    }
    task_.store(nullptr);
    vTaskDelete(nullptr);
}

bool WatchTimeService::IsTimeValid(time_t value) {
    struct tm local = {};
    localtime_r(&value, &local);
    return local.tm_year >= 125;
}
