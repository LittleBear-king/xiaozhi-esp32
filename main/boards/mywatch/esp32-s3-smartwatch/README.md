# MyWatch ESP32-S3 Smartwatch

This is the independent smartwatch product variant based on the Waveshare
ESP32-S3-Touch-AMOLED-2.06 hardware.

The initial version keeps the proven display, touch, audio, battery, and power
management behavior of the upstream board implementation. Watch-specific UI,
RTC, IMU, notifications, and power management will be developed in this
directory without changing the upstream Waveshare board identity.

Build with ESP-IDF 6.0.1 or newer:

```sh
python3 scripts/build.py mywatch/esp32-s3-smartwatch \
    --name esp32-s3-smartwatch \
    --language zh-CN \
    --wake-word nihaoxiaozhi
```
