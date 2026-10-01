#include "application.h"
#include "button.h"
#include "codecs/box_audio_codec.h"
#include "config.h"
#include "controller/watch_controller.h"
#include "hal/watch_backlight.h"
#include "hal/watch_motion.h"
#include "hal/watch_power.h"
#include "hal/watch_rtc.h"
#include "mcp_server.h"
#include "model/watch_model.h"
#include "services/watch_health_service.h"
#include "services/watch_ble_companion_service.h"
#include "services/watch_motion_service.h"
#include "services/watch_notification_service.h"
#include "services/watch_phone_service.h"
#include "services/watch_power_policy.h"
#include "services/watch_reliability_service.h"
#include "services/watch_settings_service.h"
#include "services/watch_time_service.h"
#include "ui/watch_display.h"
#include "wifi_board.h"

#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_sh8601.h>
#include <esp_lcd_touch_ft5x06.h>
#include <esp_log.h>
#include <esp_lvgl_port.h>
#include <lvgl.h>

#include <cstdio>
#include <memory>
#include <string>
#include <utility>

#define TAG "MyWatchBoard"

static const sh8601_lcd_init_cmd_t vendor_specific_init[] = {
    // set display to qspi mode
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0xC4, (uint8_t[]){0x80}, 1, 0},
    {0x44, (uint8_t[]){0x01, 0xD1}, 2, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
    {0x53, (uint8_t[]){0x20}, 1, 10},
    {0x63, (uint8_t[]){0xFF}, 1, 10},
    {0x51, (uint8_t[]){0x00}, 1, 10},
    {0x2A, (uint8_t[]){0x00, 0x16, 0x01, 0xAF}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x01, 0xF5}, 4, 0},
    {0x29, (uint8_t[]){0x00}, 0, 10},
    {0x51, (uint8_t[]){0xFF}, 1, 0},
};

class MyWatchBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_ = nullptr;
    WatchPower* power_ = nullptr;
    WatchModel watch_model_;
    WatchController watch_controller_;
    WatchHealthService health_service_;
    WatchNotificationService notification_service_;
    WatchSettingsService settings_service_;
    WatchReliabilityService reliability_service_;
    WatchPhoneService phone_service_;
    std::unique_ptr<WatchBleCompanionService> ble_companion_service_;
    Button boot_button_;
    WatchDisplay* display_ = nullptr;
    WatchBacklight* backlight_ = nullptr;
    std::unique_ptr<WatchMotion> motion_;
    std::unique_ptr<WatchMotionService> motion_service_;
    std::unique_ptr<WatchRtc> rtc_;
    std::unique_ptr<WatchTimeService> time_service_;
    std::unique_ptr<WatchPowerPolicy> power_policy_;

    void InitializePowerPolicy() {
        WatchPowerPolicy::Config power_config;
        power_policy_ = std::make_unique<WatchPowerPolicy>(*display_, *backlight_, power_config,
                                                           [this]() { power_->PowerOff(); });
        power_policy_->Start(power_->IsDischarging());
        watch_controller_.AttachPowerPolicy(*power_policy_);
    }

    void InitializeTime() {
        rtc_ = std::make_unique<WatchRtc>(i2c_bus_, RTC_I2C_ADDRESS);
        time_service_ = std::make_unique<WatchTimeService>(*rtc_);
        time_service_->SetAlarmProvider(
            [this](int hour, int minute) {
                const auto settings = settings_service_.GetSnapshot();
                return settings.alarm_enabled && settings.alarm_hour == hour &&
                       settings.alarm_minute == minute;
            },
            [this]() {
                notification_service_.Push("闹钟", "时间到了", "请查看今天的安排");
                watch_controller_.NotifyUserActivity();
                if (display_ != nullptr && !settings_service_.GetSnapshot().do_not_disturb) {
                    Application::GetInstance().Schedule(
                        [this]() { display_->ShowNotification("闹钟：时间到了", 5000); });
                }
            });
        if (!time_service_->Start()) {
            ESP_LOGW(TAG, "Time service failed to start");
        }
    }

    void InitializeSettings() {
        settings_service_.SetChangedCallback([this](const WatchSettingsSnapshot& settings) {
            power_policy_->SetRaiseToWakeEnabled(settings.raise_to_wake);
            backlight_->SetBrightness(settings.brightness, true);
        });
    }

    void InitializePhoneTransport() {
        ble_companion_service_ = std::make_unique<WatchBleCompanionService>(phone_service_);
        if (!ble_companion_service_->Start()) {
            ESP_LOGE(TAG, "BLE phone companion service failed to start");
            ble_companion_service_.reset();
        }
    }

    void InitializeMotion() {
        motion_ = std::make_unique<WatchMotion>(i2c_bus_, IMU_I2C_ADDRESS);
        if (!motion_->Initialize()) {
            ESP_LOGW(TAG, "Raise-to-wake disabled because the IMU is unavailable");
            motion_.reset();
            return;
        }

        WatchMotionService::Config motion_config;
        health_service_.SetSensorAvailable(true);
        motion_service_ = std::make_unique<WatchMotionService>(
            *motion_, motion_config, [this]() { watch_controller_.NotifyUserActivity(); },
            [this](const WatchAcceleration& acceleration, int64_t now_us) {
                health_service_.ProcessAcceleration(acceleration, now_us);
            });
        if (!motion_service_->Start()) {
            ESP_LOGW(TAG, "Raise-to-wake service failed to start");
            motion_service_.reset();
            motion_.reset();
            return;
        }
        power_policy_->AttachMotionService(motion_service_.get());
    }

    void InitializeCodecI2c() {
        // Initialize I2C peripheral
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .flags =
                {
                    .enable_internal_pullup = 1,
                },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
    }

    void InitializePower() {
        ESP_LOGI(TAG, "Initialize AXP2101 power management");
        power_ = new WatchPower(i2c_bus_, 0x34);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.sclk_io_num = DISPLAY_QSPI_PCLK_PIN;
        buscfg.data0_io_num = DISPLAY_QSPI_DATA0_PIN;
        buscfg.data1_io_num = DISPLAY_QSPI_DATA1_PIN;
        buscfg.data2_io_num = DISPLAY_QSPI_DATA2_PIN;
        buscfg.data3_io_num = DISPLAY_QSPI_DATA3_PIN;
        buscfg.data4_io_num = GPIO_NUM_NC;
        buscfg.data5_io_num = GPIO_NUM_NC;
        buscfg.data6_io_num = GPIO_NUM_NC;
        buscfg.data7_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        buscfg.flags = SPICOMMON_BUSFLAG_QUAD;
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            watch_controller_.RequestTalk();
        });

#if CONFIG_USE_DEVICE_AEC
        boot_button_.OnDoubleClick([this]() {
            watch_controller_.NotifyUserActivity();
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateIdle) {
                app.SetAecMode(app.GetAecMode() == kAecOff ? kAecOnDeviceSide : kAecOff);
            }
        });
#endif
    }

    void InitializeSH8601Display() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;

        // Initialize the panel transport.
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_QSPI_CS_PIN;
        io_config.dc_gpio_num = GPIO_NUM_NC;
        io_config.spi_mode = 0;
        io_config.pclk_hz = 40 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 32;
        io_config.lcd_param_bits = 8;
        io_config.flags.quad_mode = true;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        // Initialize the SH8601 panel.
        ESP_LOGD(TAG, "Install LCD driver");
        const sh8601_vendor_config_t vendor_config = {
            .init_cmds = &vendor_specific_init[0],
            .init_cmds_size = sizeof(vendor_specific_init) / sizeof(sh8601_lcd_init_cmd_t),
            .flags = {
                .use_qspi_interface = 1,
            }};

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RESET_PIN;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        panel_config.vendor_config = (void*)&vendor_config;
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(panel_io, &panel_config, &panel));
        esp_lcd_panel_set_gap(panel, 0x16, 0);
        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, false);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        esp_lcd_panel_disp_on_off(panel, true);
        display_ = new WatchDisplay(
            panel_io, panel, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y,
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY, watch_model_, watch_controller_,
            health_service_, notification_service_, settings_service_, *time_service_,
            phone_service_, reliability_service_);
        backlight_ = new WatchBacklight(panel_io, display_);
        backlight_->RestoreBrightness();
    }

    void InitializeTouch() {
        esp_lcd_touch_handle_t tp;
        esp_lcd_touch_config_t tp_cfg = {
            .x_max = DISPLAY_WIDTH - 1,
            .y_max = DISPLAY_HEIGHT - 1,
            .rst_gpio_num = TOUCH_RESET_PIN,
            .int_gpio_num = TOUCH_INTERRUPT_PIN,
            .levels =
                {
                    .reset = 0,
                    .interrupt = 0,
                },
            .flags =
                {
                    .swap_xy = 0,
                    .mirror_x = 0,
                    .mirror_y = 0,
                },
        };
        esp_lcd_panel_io_handle_t tp_io_handle = NULL;
        esp_lcd_panel_io_i2c_config_t tp_io_config = {
            .dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS,
            .control_phase_bytes = 1,
            .dc_bit_offset = 0,
            .lcd_cmd_bits = 8,
            .flags = {
                .disable_control_phase = 1,
            }};
        tp_io_config.scl_speed_hz = 400 * 1000;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_, &tp_io_config, &tp_io_handle));
        ESP_LOGI(TAG, "Initialize touch controller");
        ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp));
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = lv_display_get_default(),
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
        ESP_LOGI(TAG, "Touch panel initialized successfully");
    }

    // Initialize device tools.
    void InitializeTools() {
        auto& mcp_server = McpServer::GetInstance();
        mcp_server.AddTool("self.system.reconfigure_wifi",
                           "End this conversation and enter WiFi configuration mode.\n"
                           "**CAUTION** You must ask the user to confirm this action.",
                           PropertyList(), [this](const PropertyList& properties) {
                               EnterWifiConfigMode();
                               return true;
                           });
        mcp_server.AddTool("self.watch.get_activity",
                           "Get today's step, distance, calorie, and active-minute summary.",
                           PropertyList(), [this](const PropertyList& properties) -> ReturnValue {
                               const auto value = health_service_.GetSnapshot();
                               char result[160];
                               snprintf(result, sizeof(result),
                                        "{\"steps\":%lu,\"distance_m\":%lu,\"calories_tenths\":%lu,"
                                        "\"active_minutes\":%lu}",
                                        static_cast<unsigned long>(value.steps),
                                        static_cast<unsigned long>(value.distance_m),
                                        static_cast<unsigned long>(value.calories_tenths),
                                        static_cast<unsigned long>(value.active_minutes));
                               return std::string(result);
                           });
        mcp_server.AddTool(
            "self.watch.add_notification", "Add a short item to the watch notification center.",
            PropertyList({Property("source", kPropertyTypeString),
                          Property("title", kPropertyTypeString),
                          Property("body", kPropertyTypeString)}),
            [this](const PropertyList& properties) -> ReturnValue {
                const auto& source = properties["source"].value<std::string>();
                const auto& title = properties["title"].value<std::string>();
                const auto& body = properties["body"].value<std::string>();
                notification_service_.Push(source.c_str(), title.c_str(), body.c_str());
                return true;
            });
    }

public:
    MyWatchBoard()
        : watch_controller_(watch_model_),
          phone_service_(notification_service_),
          boot_button_(BOOT_BUTTON_GPIO) {
        InitializeCodecI2c();
        InitializePower();
        InitializeTime();
        watch_controller_.UpdateBattery(power_->GetBatteryLevel(), power_->IsCharging(),
                                        power_->IsDischarging());
        InitializeSpi();
        InitializeSH8601Display();
        InitializeTouch();
        InitializeButtons();
        InitializeTools();
        InitializePhoneTransport();
        InitializePowerPolicy();
        InitializeMotion();
        InitializeSettings();
    }

    virtual AudioCodec* GetAudioCodec() override {
        static BoxAudioCodec audio_codec(
            i2c_bus_, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE, AUDIO_I2S_GPIO_MCLK,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR, AUDIO_CODEC_ES7210_ADDR,
            AUDIO_INPUT_REFERENCE);
        return &audio_codec;
    }

    bool ShouldStartNetworkOnBoot() const override { return false; }
    bool CanEnterIdleWithoutNetwork() const override { return true; }
    bool ShouldUseDeviceAec() const override { return false; }

    virtual Display* GetDisplay() override { return display_; }

    virtual Backlight* GetBacklight() override { return backlight_; }

    bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        charging = power_->IsCharging();
        discharging = power_->IsDischarging();
        level = power_->GetBatteryLevel();
        watch_controller_.UpdateBattery(level, charging, discharging);
        return true;
    }

    void SetNetworkEventCallback(NetworkEventCallback callback) override {
        WifiBoard::SetNetworkEventCallback(
            [this, callback = std::move(callback)](NetworkEvent event, const std::string& data) {
                watch_controller_.HandleNetworkEvent(event);
                if (callback) {
                    callback(event, data);
                }
            });
    }

    virtual void SetPowerSaveLevel(PowerSaveLevel level) override {
        if (level != PowerSaveLevel::LOW_POWER) {
            watch_controller_.NotifyUserActivity();
        }
        WifiBoard::SetPowerSaveLevel(level);
    }
};

DECLARE_BOARD(MyWatchBoard);
