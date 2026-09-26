#ifndef _MYWATCH_WATCH_BACKLIGHT_H_
#define _MYWATCH_WATCH_BACKLIGHT_H_

#include "backlight.h"

#include <esp_lcd_panel_io.h>

class Display;

class WatchBacklight final : public Backlight {
public:
    WatchBacklight(esp_lcd_panel_io_handle_t panel_io, Display* display)
        : panel_io_(panel_io), display_(display) {}

protected:
    void SetBrightnessImpl(uint8_t brightness) override;

private:
    esp_lcd_panel_io_handle_t panel_io_;
    Display* display_;
};

#endif  // _MYWATCH_WATCH_BACKLIGHT_H_
