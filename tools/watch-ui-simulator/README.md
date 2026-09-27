# MyWatch UI Simulator

Open `index.html` directly in a browser. The simulator has no package manager,
build step, network dependency, or backend.

The simulator models the product states that affect the watch display:

- idle watch face
- assistant connection
- listening
- speaking
- notification
- error

Use it to review layout, content hierarchy, colors, and transitions before
implementing a screen in LVGL. Firmware behavior remains authoritative. When a
design is accepted, update the values in
`main/boards/mywatch/esp32-s3-smartwatch/ui/watch_ui_tokens.h` and the simulator
CSS together in the same change.
