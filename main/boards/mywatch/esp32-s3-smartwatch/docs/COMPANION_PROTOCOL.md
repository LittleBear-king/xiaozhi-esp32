# MyWatch Companion Protocol

The companion boundary is transport independent. BLE GATT, Wi-Fi, and a phone
debug bridge must deliver complete UTF-8 JSON messages to
`WatchPhoneService::HandleMessage`. The service validates size and required
fields before data reaches the notification store.

## Limits

- message: 768 bytes
- source: 23 bytes
- title: 95 bytes
- body: 159 bytes
- retained notifications: 12, newest first

## Notification

```json
{
  "type": "notification",
  "source": "Calendar",
  "title": "Design review",
  "body": "Starts in 10 minutes"
}
```

The phone app should filter private or blocked applications before forwarding a
message. The watch applies its own do-not-disturb presentation policy and still
retains accepted notifications in the notification application.

## Transport roadmap

The ESP32-S3 product currently builds with Wi-Fi and Bluetooth disabled. The
next companion milestone adds a board-specific NimBLE GATT transport with
pairing, bonding, message framing, reconnect backoff, and find-phone commands.
Keeping the JSON boundary independent means the notification store and UI do not
change when that transport is enabled.

## Remaining phone integration

The following features intentionally stop at the companion boundary until phone debugging:

- weather snapshots for the tools/weather surface
- media title, artist, and playback state
- find-phone request and acknowledgement

These messages must use the same bounded UTF-8 framing and connection state as
notifications. They must not be coupled to LVGL or the radio driver; the phone
service remains the parser and state owner.
