# MyWatch ESP32-S3 Smartwatch

MyWatch is an independent smartwatch product variant based on the Waveshare
ESP32-S3-Touch-AMOLED-2.06 hardware. It reuses Xiaozhi voice, networking, audio,
and protocol services while keeping product-specific hardware and UI in this
directory.

## Structure

- `smartwatch_board.cc`: hardware composition and generic `Board` capabilities
- `watch_power.*`: AXP2101 rail and charging configuration
- `watch_backlight.*`: SH8601 AMOLED brightness transport
- `watch_display.*`: Xiaozhi state to watch-surface adapter
- `watch_face.*`: idle LVGL surface, clock, and talk action
- `watch_ui_tokens.h`: firmware visual constants
- `ARCHITECTURE.md`: ownership rules and feature extension plan

Runtime flow:

```text
Idle watch face -> tap AI -> Connecting -> Listening -> Speaking -> Idle watch face
```

The AI action uses `Application::ToggleChatState()`. Audio capture, AEC, protocol
transport, wake-word detection, and playback remain owned by Xiaozhi core.

## Build

Use ESP-IDF 6.0.1 or newer:

```sh
python3 scripts/build.py mywatch/esp32-s3-smartwatch \
    --name esp32-s3-smartwatch \
    --language zh-CN \
    --wake-word nihaoxiaozhi
```

## UI Simulator

Open `tools/watch-ui-simulator/index.html` in a browser. It previews the idle,
connecting, listening, speaking, notification, and error states without an
ESP-IDF build.
