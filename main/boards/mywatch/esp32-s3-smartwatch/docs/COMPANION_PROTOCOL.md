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

The firmware exposes a NimBLE GATT service using Nordic UART-style 128-bit UUIDs:

| Attribute | UUID | Properties | Purpose |
| --- | --- | --- | --- |
| Service | `6e400001-b5a3-f393-e0a9-e50e24dcca9e` | Primary | MyWatch companion |
| RX | `6e400002-b5a3-f393-e0a9-e50e24dcca9e` | Encrypted write | Phone to watch messages |
| TX | `6e400003-b5a3-f393-e0a9-e50e24dcca9e` | Read, notify | Status read and write acknowledgements |

The device advertises as `MyWatch`. BLE pairing uses LE Secure Connections,
bonding, and Just Works (no display/input passkey); RX writes require an encrypted
link. The watch accepts UTF-8 JSON lines terminated by LF (`0x0a`), including
messages split across multiple GATT writes. A complete line receives `OK\n` or
`ERR\n` on TX if notifications are enabled. A line is limited to 768 bytes.

Use nRF Connect for first-device testing: scan for `MyWatch`, connect, enable
notifications on TX, write to RX using UTF-8 text with a trailing newline, then
read TX for connection and message counters. The notification center should show
the accepted source and title. Bonding is stored by NimBLE in NVS.

Example RX value (include a final LF byte):

```json
{"type":"notification","source":"Calendar","title":"Design review","body":"Starts in 10 minutes"}
```

The TX read value is `connected=<0|1>;received=<count>;rejected=<count>`. TX
notifications return one `OK\n` or `ERR\n` for each complete JSON line.

## Remaining phone integration

The following features intentionally stop at the companion boundary until phone debugging:

- weather snapshots for the tools/weather surface
- media title, artist, and playback state
- find-phone request and acknowledgement

These message types and actions still need implementation and phone-side testing.
They must use the same bounded UTF-8 framing and connection state as notifications.
