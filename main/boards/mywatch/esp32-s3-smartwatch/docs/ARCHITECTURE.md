# MyWatch Architecture

## Goals

MyWatch is a product variant, not a collection of patches on the upstream
Waveshare board. Its code should keep hardware access, product behavior, and UI
rendering independently replaceable.

The dependency direction is:

```text
Application state and Xiaozhi services
                  |
                  v
        WatchDisplay adapter
          |              |
          v              v
     WatchFace       Assistant UI

MyWatchBoard -> display / touch / audio / power / buttons
```

Lower layers must not depend on a concrete higher layer. A sensor driver does
not update LVGL directly, and a view does not read an I2C device directly.

## Directory Rules

| Directory | Owns | Must not own |
| --- | --- | --- |
| `board/` | device composition, bus setup, callback wiring | widget layouts or feature algorithms |
| `hal/` | product-specific device and transport adapters | application state or LVGL screens |
| `ui/` | LVGL surfaces, display state adaptation, visual tokens | direct I2C/SPI sensor access |
| `services/` | future battery, motion, time, notification models | board pin definitions or widgets |
| `docs/` | architecture and product engineering decisions | generated output |

The board root is reserved for `config.h`, `config.json`, `board.cmake`, and
`README.md`. Add every compiled source explicitly to `board.cmake`. Create
`services/` when the first product service is implemented; do not keep feature
implementations in the root directory.

## Current Components

### `board/smartwatch_board.cc`

The composition root for the product. It creates hardware components, connects
callbacks, and implements the generic `Board` capabilities. It owns pin and bus
knowledge but contains no watch-face layout.

### `hal/watch_power.*`

AXP2101 rail and charger policy for this product. Register values live here so
future battery and charging changes do not affect board composition.

### `hal/watch_backlight.*`

SH8601 brightness transport. It converts the generic `Backlight` percentage to
the panel command.

### `ui/watch_display.*`

Adapter between Xiaozhi display events and MyWatch surfaces. It decides whether
the watch face or the existing assistant UI is visible. It does not create
watch-face widgets.

### `ui/watch_face.*`

Owns the idle LVGL object tree, clock refresh, and touch action. It reports
actions through callbacks and has no dependency on `Application`.

### `ui/watch_ui_tokens.h`

Firmware-side visual constants. New watch faces should keep dimensions, colors,
and user-facing labels out of display orchestration code.

## Runtime Flow

```text
Idle -> WatchFace -> tap AI -> ToggleChatState
     -> Connecting -> Listening -> Speaking
     -> session closes -> Idle -> WatchFace
```

Notifications and errors temporarily use the assistant surface. When the
notification timer expires and the application is idle, `WatchDisplay` restores
the watch face.

## Adding Product Features

Add each feature behind a small service interface with one owner:

| Feature | Suggested owner | UI input |
| --- | --- | --- |
| Battery history | `watch_battery_service.*` | immutable battery snapshot |
| IMU and steps | `watch_motion_service.*` | step/activity snapshot |
| RTC/time zones | `watch_time_service.*` | local clock snapshot |
| Phone notifications | `watch_notification_service.*` | bounded notification model |
| Sleep policy | `watch_power_policy.*` | activity and charging events |

Services publish compact snapshots or events. Views render those values. The
board wires the services together and should not become their implementation.

Long-running work belongs in dedicated FreeRTOS tasks with bounded queues.
Application mutations must be scheduled onto the application task. LVGL object
changes must run while the display lock is held.

## UI Simulation

`tools/watch-ui-simulator/index.html` is the fast product-design preview. It is
dependency-free and models the display states without ESP-IDF hardware.

The simulator is a visual contract, while LVGL firmware is the runtime source
of truth. Any accepted change to the shared watch-face design should update both
`ui/watch_ui_tokens.h`/`ui/watch_face.cc` and the simulator in one commit.
