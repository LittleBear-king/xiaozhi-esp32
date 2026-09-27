#include "watch_backlight.h"

#include "display.h"

namespace {
constexpr uint32_t kLcdWriteCommandOpcode = 0x02U;
constexpr int kBrightnessCommand = 0x51;
}  // namespace

void WatchBacklight::SetBrightnessImpl(uint8_t brightness) {
    DisplayLockGuard lock(display_);

    uint8_t data[] = {static_cast<uint8_t>((255 * brightness) / 100)};
    const int command =
        static_cast<int>(((kBrightnessCommand & 0xFF) << 8) | (kLcdWriteCommandOpcode << 24));
    esp_lcd_panel_io_tx_param(panel_io_, command, data, sizeof(data));
}
