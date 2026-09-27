#include "watch_motion.h"

#include <esp_log.h>
#include <qmi8658.h>

#define TAG "WatchMotion"

struct WatchMotion::Impl {
    Impl(i2c_master_bus_handle_t bus, uint8_t device_address)
        : i2c_bus(bus), address(device_address) {}

    i2c_master_bus_handle_t i2c_bus;
    uint8_t address;
    qmi8658_dev_t device = {};
    bool initialized = false;
};

WatchMotion::WatchMotion(i2c_master_bus_handle_t i2c_bus, uint8_t address)
    : impl_(std::make_unique<Impl>(i2c_bus, address)) {}

WatchMotion::~WatchMotion() {
    if (impl_->device.dev_handle != nullptr) {
        qmi8658_enable_sensors(&impl_->device, QMI8658_DISABLE_ALL);
        i2c_master_bus_rm_device(impl_->device.dev_handle);
    }
}

bool WatchMotion::Initialize() {
    if (impl_->initialized) {
        return true;
    }

    esp_err_t result = qmi8658_init(&impl_->device, impl_->i2c_bus, impl_->address);
    if (result != ESP_OK) {
        ESP_LOGW(TAG, "QMI8658 initialization failed: %s", esp_err_to_name(result));
        return false;
    }

    result = qmi8658_set_accel_range(&impl_->device, QMI8658_ACCEL_RANGE_2G);
    if (result == ESP_OK) {
        result = qmi8658_set_accel_odr(&impl_->device, QMI8658_ACCEL_ODR_LOWPOWER_21HZ);
    }
    if (result == ESP_OK) {
        result = qmi8658_enable_sensors(&impl_->device, QMI8658_ENABLE_ACCEL);
    }
    if (result != ESP_OK) {
        ESP_LOGW(TAG, "QMI8658 low-power configuration failed: %s", esp_err_to_name(result));
        return false;
    }

    qmi8658_set_accel_unit_mg(&impl_->device, true);
    impl_->initialized = true;
    ESP_LOGI(TAG, "QMI8658 accelerometer ready at 0x%02x", impl_->address);
    return true;
}

bool WatchMotion::ReadAcceleration(WatchAcceleration& acceleration) {
    if (!impl_->initialized) {
        return false;
    }

    bool ready = false;
    if (qmi8658_is_data_ready(&impl_->device, &ready) != ESP_OK || !ready) {
        return false;
    }

    return qmi8658_read_accel_mg(&impl_->device, &acceleration.x_mg, &acceleration.y_mg,
                                 &acceleration.z_mg) == ESP_OK;
}
