# MyWatch ESP32-S3 Smartwatch

This is the independent smartwatch product variant based on the Waveshare
ESP32-S3-Touch-AMOLED-2.06 hardware.

The initial version keeps the proven display, touch, audio, battery, and power
management behavior of the upstream board implementation. Watch-specific UI,
RTC, IMU, notifications, and power management will be developed in this
directory without changing the upstream Waveshare board identity.

## Architecture

- MyWatchBoard is the hardware adaptation layer. It initializes the SH8601
  AMOLED panel, FT5x06 touch controller, audio codec, AXP2101 PMIC, buttons, and
  power-save timer.
- WatchDisplay is the smartwatch presentation layer. It extends the existing
  SpiLcdDisplay, keeps the upstream assistant UI, and adds an AMOLED-friendly
  idle watch face.
- Application remains the product state machine. Idle state shows the watch
  face; listening, speaking, network, error, and notification states use the
  existing assistant UI.
- The round AI button posts Application::StartListening(). Audio capture,
  protocol transport, wake-word detection, and playback continue to use the
  upstream Xiaozhi services.

Current UI flow:

Idle watch face -> tap AI -> Listening -> Speaking -> Idle watch face

Future watch services such as RTC synchronization, IMU/step counting, phone
notifications, and deeper sleep management should be added as independent
components and exposed to the board/display layer through small interfaces.

Build with ESP-IDF 6.0.1 or newer:

```sh
python3 scripts/build.py mywatch/esp32-s3-smartwatch \
    --name esp32-s3-smartwatch \
    --language zh-CN \
    --wake-word nihaoxiaozhi
```
