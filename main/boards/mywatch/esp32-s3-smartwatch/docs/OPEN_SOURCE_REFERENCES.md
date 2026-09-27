# Open-source smartwatch architecture references

MyWatch uses architecture ideas from established smartwatch projects. This file
records the design decisions so later changes remain intentional. No source code
was copied from these projects.

## InfiniTime

Reference: <https://github.com/InfiniTimeOrg/InfiniTime>

Useful patterns:

- hardware drivers, controllers, display screens, and system tasks have separate
  owners;
- the display application owns screen lifecycle instead of letting drivers
  mutate widgets;
- system events cross task boundaries through controlled dispatch points.

Applied in MyWatch:

- `hal/` owns device transport;
- `services/` owns long-lived product behavior;
- `WatchDisplay` owns the assistant/watch-face surface transition;
- `WatchController` schedules application mutations on the Xiaozhi application
  task.

## ZSWatch

Reference: <https://github.com/ZSWatch/ZSWatch>

Useful patterns:

- applications, events, managers, sensors, and UI are distinct modules;
- services publish state and events instead of directly controlling every
  consumer;
- settings, retained state, diagnostics, and watchdog support are product
  subsystems rather than screen code.

Applied in MyWatch:

- `WatchModel` publishes a compact thread-safe snapshot;
- motion detection emits a semantic wrist-raise event;
- `WatchPowerPolicy` decides whether that event should wake the display;
- board callbacks do not directly update LVGL.

## Open-Smartwatch OS

Reference: <https://github.com/Open-Smartwatch/open-smartwatch-os>

Useful patterns:

- HAL, services, applications, overlays, and UI are separate;
- the desktop emulator is part of the normal UI workflow;
- application interfaces appear when multiple independent watch apps exist.

Applied in MyWatch:

- firmware visual tokens and `tools/watch-ui-simulator` are updated together;
- hardware-independent behavior stays outside `board/`;
- an app registry and generic screen router are deferred until a second native
  watch app exists, avoiding an empty framework with no real consumers.

## Product architecture roadmap

### Foundation

- board-specific HAL and composition root;
- model/controller/view split;
- bounded motion task and centralized power policy;
- UI simulator with the same safe-area tokens as firmware.

### Native watch applications

Add `apps/` and a screen router when Settings, Activity, or Notifications becomes
an independent screen. Each app will own lifecycle methods (`OnEnter`, `OnExit`,
`OnEvent`) and will receive typed model snapshots rather than hardware handles.

### Product services

Add bounded services for time/RTC, health and steps, notifications, settings,
and phone connectivity. Cross-task delivery should use a fixed-size typed queue.
Do not introduce a global event bus until at least three producers and consumers
need it.

### Production readiness

Before a product release, add OTA rollback validation, watchdog ownership,
crash-reason persistence, battery profiling, manufacturing diagnostics, settings
migration, localization, accessibility review, and hardware-in-the-loop smoke
checks.
