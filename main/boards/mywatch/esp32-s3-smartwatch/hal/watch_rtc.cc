#include "watch_rtc.h"

#include <esp_log.h>

#define TAG "WatchRtc"

namespace {
constexpr uint8_t kTimeRegister = 0x04;
constexpr uint8_t kVoltageLowFlag = 0x80;
}  // namespace

WatchRtc::WatchRtc(i2c_master_bus_handle_t bus, uint8_t address) : bus_(bus), address_(address) {}

WatchRtc::~WatchRtc() {
    if (device_ != nullptr) {
        i2c_master_bus_rm_device(device_);
    }
}

bool WatchRtc::Initialize() {
    if (device_ != nullptr) {
        return true;
    }
    i2c_device_config_t config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address_,
        .scl_speed_hz = 400000,
    };
    const esp_err_t result = i2c_master_bus_add_device(bus_, &config, &device_);
    if (result != ESP_OK) {
        ESP_LOGW(TAG, "PCF85063 device unavailable: %s", esp_err_to_name(result));
        device_ = nullptr;
        return false;
    }

    uint8_t probe = 0;
    const uint8_t control_register = 0x00;
    if (i2c_master_transmit_receive(device_, &control_register, 1, &probe, 1, 50) != ESP_OK) {
        ESP_LOGW(TAG, "PCF85063 did not respond at 0x%02x", address_);
        i2c_master_bus_rm_device(device_);
        device_ = nullptr;
        return false;
    }
    ESP_LOGI(TAG, "PCF85063 RTC ready at 0x%02x", address_);
    return true;
}

bool WatchRtc::Read(struct tm& local_time) {
    if (device_ == nullptr)
        return false;
    uint8_t data[7] = {};
    const uint8_t reg = kTimeRegister;
    if (i2c_master_transmit_receive(device_, &reg, 1, data, sizeof(data), 100) != ESP_OK ||
        (data[0] & kVoltageLowFlag) != 0) {
        return false;
    }
    local_time = {};
    local_time.tm_sec = FromBcd(data[0] & 0x7F);
    local_time.tm_min = FromBcd(data[1] & 0x7F);
    local_time.tm_hour = FromBcd(data[2] & 0x3F);
    local_time.tm_mday = FromBcd(data[3] & 0x3F);
    local_time.tm_wday = FromBcd(data[4] & 0x07);
    local_time.tm_mon = FromBcd(data[5] & 0x1F) - 1;
    local_time.tm_year = FromBcd(data[6]) + 100;
    local_time.tm_isdst = -1;
    return local_time.tm_year >= 125 && local_time.tm_mon >= 0 && local_time.tm_mon < 12 &&
           local_time.tm_mday >= 1 && local_time.tm_mday <= 31;
}

bool WatchRtc::Write(const struct tm& local_time) {
    if (device_ == nullptr || local_time.tm_year < 100 || local_time.tm_year > 199)
        return false;
    uint8_t data[8] = {
        kTimeRegister,
        ToBcd(local_time.tm_sec),
        ToBcd(local_time.tm_min),
        ToBcd(local_time.tm_hour),
        ToBcd(local_time.tm_mday),
        ToBcd(local_time.tm_wday),
        ToBcd(local_time.tm_mon + 1),
        ToBcd(local_time.tm_year - 100),
    };
    return i2c_master_transmit(device_, data, sizeof(data), 100) == ESP_OK;
}

uint8_t WatchRtc::ToBcd(int value) {
    return static_cast<uint8_t>(((value / 10) << 4) | (value % 10));
}

int WatchRtc::FromBcd(uint8_t value) { return (value >> 4) * 10 + (value & 0x0F); }
