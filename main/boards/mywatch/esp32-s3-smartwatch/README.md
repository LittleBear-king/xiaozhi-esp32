# MyWatch ESP32-S3 Smartwatch

MyWatch is an independent smartwatch product variant based on the Waveshare
ESP32-S3-Touch-AMOLED-2.06 hardware. It reuses Xiaozhi voice, networking, audio,
and protocol services while keeping product-specific hardware and UI in this
directory.

## Structure

- `board/`: hardware composition and generic `Board` capabilities
- `controller/`: product actions and platform-event adaptation
- `model/`: thread-safe state snapshots consumed by the UI
- `hal/`: product-specific power and AMOLED transport
- `ui/`: display adapter, watch surfaces, and visual constants
- `docs/`: architecture and product engineering notes
- `board.cmake`: explicit source manifest for the nested modules
- `config.h` / `config.json`: board pins, identity, and build variants

See `docs/ARCHITECTURE.md` for ownership rules and the extension plan.

Runtime flow:

```text
Idle watch face -> tap AI -> Connecting -> Listening -> Speaking -> Idle watch face
```

The AI action is scheduled through `WatchController`. Audio capture, AEC, protocol
transport, wake-word detection, and playback remain owned by Xiaozhi core.

## Current Baseline

- local time, calendar date, and weekday
- live battery percentage with charging and low-battery states
- Wi-Fi offline, connecting, and online states
- touch and hardware-button assistant entry
- charging-aware display sleep policy
- thread-safe `WatchModel` snapshots between platform callbacks and LVGL

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
