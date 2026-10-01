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
       /          |             \
      v           v              v
 WatchFace   WatchAppRouter   Assistant UI
                  |
                  v
        Independent watch apps

MyWatchBoard -> display / touch / audio / power / buttons
      |
      +-> WatchMotion HAL -> WatchMotionService
                                 |
                                 v
                         WatchController
                                 |
                                 v
                         WatchPowerPolicy
```

Lower layers must not depend on a concrete higher layer. A sensor driver does
not update LVGL directly, and a view does not read an I2C device directly.

## Directory Rules

| Directory | Owns | Must not own |
| --- | --- | --- |
| `board/` | device composition, bus setup, callback wiring | widget layouts or feature algorithms |
| `apps/` | application IDs, navigation stack, and lifecycle contract | widget layout or hardware access |
| `controller/` | user actions and platform-event adaptation | LVGL object ownership or register access |
| `model/` | thread-safe product state snapshots | hardware access or view logic |
| `hal/` | product-specific device and transport adapters | application state or LVGL screens |
| `ui/` | LVGL surfaces, application views, display adaptation, visual tokens | direct I2C/SPI sensor access |
| `services/` | power policy, motion, time, and notification behavior | board pin definitions or widgets |
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

### `hal/watch_motion.*`

QMI8658C transport adapter. It owns sensor initialization and acceleration reads
on the shared board I2C bus. Sensor register and third-party component details do
not escape this layer.

### `services/watch_motion_service.*`

Owns the raise-to-wake algorithm and its bounded sampling task. It filters the
absolute screen-normal acceleration so either sensor Z polarity works, recognizes
a lowered-to-raised transition that remains stable for consecutive samples,
applies a cooldown, and publishes a callback without touching LVGL or power hardware.

### `services/watch_power_policy.*`

Owns display timeout, saved-brightness restoration, charging behavior, optional
shutdown, and motion-service lifecycle. Hardware and network callbacks report
activity through `WatchController`; the policy remains the only product module
that decides when the display sleeps or wakes.

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

### `model/watch_model.*`

Owns the atomic battery and network snapshot shared between platform callbacks
and the LVGL task. Views only consume snapshots and never query hardware.

### `controller/watch_controller.*`

Normalizes battery and network events, schedules assistant and user-activity
actions on the application task, and exposes application state to the display
adapter. Cross-task callbacks must enter product behavior through this layer.


### Application runtime

`WatchAppRouter` owns a fixed registry and bounded back stack. Applications are
created lazily on the LVGL task and receive `Create`, `OnResume`, `OnPause`,
and `OnTick` events. They consume service snapshots and navigate only through
`WatchAppNavigator`. The first applications are launcher, activity,
notifications, settings, and tools.

### Product services

- `WatchTimeService` restores time from PCF85063 and writes synchronized time back.
- `WatchHealthService` turns the shared QMI8658 stream into bounded daily activity data.
- `WatchNotificationService` retains twelve newest notifications without unbounded queues.
- `WatchSettingsService` owns stable NVS keys in the `watch` namespace.
- `WatchPhoneService` validates transport-independent companion messages.
- `WatchBleCompanionService` owns NimBLE advertising, LE bonding, GATT framing,
  and forwards complete messages to `WatchPhoneService`.
- `WatchReliabilityService` persists boot/reset diagnostics and exposes rollback/watchdog state.

The product partition table keeps two 3.875 MiB OTA slots, an 8 MiB asset
partition, and a 128 KiB coredump partition. ESP-IDF rollback remains owned by
the existing OTA service; MyWatch records reset context and does not mark an
image valid early.

## Runtime Flow

```text
Idle -> WatchFace -> tap AI -> WatchController::RequestTalk
     -> Connecting -> Listening -> Speaking
     -> session closes -> Idle -> WatchFace
```

Notifications and errors temporarily use the assistant surface. When the
notification timer expires and the application is idle, `WatchDisplay` restores
the watch face.

After the display timeout, AMOLED brightness reaches zero. `WatchMotionService`
continues with the QMI8658C accelerometer in low-power mode. A valid wrist raise
is scheduled onto the application task, which wakes `PowerSaveTimer` and restores
the saved user brightness. Thresholds remain in the service configuration so
they can be tuned from physical-device traces without changing the HAL.

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

## External references

See `OPEN_SOURCE_REFERENCES.md` for the InfiniTime, ZSWatch, and Open-Smartwatch
patterns used here and the staged product architecture roadmap.
