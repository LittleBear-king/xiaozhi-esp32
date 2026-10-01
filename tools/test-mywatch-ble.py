#!/usr/bin/env python3
"""Send one companion notification to a paired MyWatch over BLE."""

import argparse
import asyncio

from bleak import BleakClient, BleakScanner

RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"


async def run(address: str, source: str, title: str, body: str) -> None:
    device = await BleakScanner.find_device_by_address(address, timeout=10)
    if device is None:
        raise RuntimeError(f"MyWatch {address} was not found. Check Bluetooth and advertising.")
    async with BleakClient(device, timeout=15) as client:
        if not client.is_connected:
            raise RuntimeError("BLE connection failed")

        def on_status(_, value: bytearray) -> None:
            print(f"TX notify: {bytes(value).decode('utf-8', errors='replace').rstrip()}")

        await client.start_notify(TX_UUID, on_status)
        payload = (
            '{"type":"notification","source":"%s","title":"%s","body":"%s"}\n'
            % (source, title, body)
        ).encode("utf-8")
        await client.write_gatt_char(RX_UUID, payload, response=True)
        status = bytes(await client.read_gatt_char(TX_UUID)).decode("utf-8", errors="replace")
        print(f"TX read: {status.rstrip()}")
        await asyncio.sleep(0.5)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--address", required=True, help="BLE address, for example 80:B5:4E:F2:04:4A")
    parser.add_argument("--source", default="Computer")
    parser.add_argument("--title", default="BLE test")
    parser.add_argument("--body", default="MyWatch companion link is working")
    args = parser.parse_args()
    asyncio.run(run(args.address, args.source, args.title, args.body))


if __name__ == "__main__":
    main()
