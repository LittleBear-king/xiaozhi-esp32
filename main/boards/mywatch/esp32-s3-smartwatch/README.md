# MyWatch ESP32-S3 Smartwatch

MyWatch is an independent smartwatch product variant based on the Waveshare
ESP32-S3-Touch-AMOLED-2.06 hardware. It reuses Xiaozhi voice, networking, audio,
and protocol services while keeping product-specific hardware and UI in this
directory.

## Structure

- `board/`: hardware composition and generic `Board` capabilities
- `controller/`: product actions and platform-event adaptation
- `model/`: thread-safe state snapshots consumed by the UI
- `hal/`: product-specific power, AMOLED, and QMI8658 transport
- `services/`: power policy, motion algorithms, and product behavior
- `apps/`: application contract, fixed-capacity router, and lifecycle
- `ui/`: display adapter, watch surfaces, application views, and visual constants
- `partitions/`: product OTA and crash-dump partition layout
- `docs/`: architecture decisions, open-source references, and product roadmap
- `board.cmake`: explicit source manifest for the nested modules
- `config.h` / `config.json`: board pins, identity, and build variants

See `docs/ARCHITECTURE.md` for ownership rules and
`docs/OPEN_SOURCE_REFERENCES.md` for reference projects and the staged roadmap.

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
- centralized charging-aware display sleep and wake policy
- low-power QMI8658 raise-to-wake detection
- rounded-screen safe-area layout shared with the UI simulator
- thread-safe `WatchModel` snapshots between platform callbacks and LVGL
- fixed-capacity application router with lifecycle callbacks and bounded history
- independent activity, notification, settings, and tools applications
- PCF85063 RTC restore and network-time writeback
- daily QMI8658 step, distance, energy, and active-minute estimates
- bounded notification center with do-not-disturb behavior
- persistent brightness, clock format, raise-to-wake, and alarm settings
- RTC-triggered alarm notifications with screen wake-up
- centisecond stopwatch and five-minute countdown tools
- OTA rollback partitions, flash coredumps, watchdog status, and reset diagnostics
- transport-neutral validated companion notification protocol
- bonded NimBLE companion GATT with encrypted writes and bounded JSON framing

The firmware-side product baseline is complete. The remaining integration work
is device/phone verification and implementing weather, media control, and
find-phone behavior on top of the companion protocol.

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
