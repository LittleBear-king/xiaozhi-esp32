#!/usr/bin/env python3
"""Exercise the MyWatch voice WebSocket with a WAV recording."""

from __future__ import annotations

import argparse
import asyncio
import json
from pathlib import Path

import websockets


async def run(args: argparse.Namespace) -> None:
    audio_path = Path(args.audio)
    output_path = Path(args.output)
    if not audio_path.is_file():
        raise SystemExit(f"Audio file not found: {audio_path}")

    async with websockets.connect(args.url, max_size=None) as socket:
        await socket.send(json.dumps({"type": "start", "language": args.language}))
        print("TX start")
        while True:
            event = json.loads(await socket.recv())
            print(f"RX {event}")
            if event.get("type") == "ready":
                break

        with audio_path.open("rb") as audio:
            while chunk := audio.read(args.chunk_size):
                await socket.send(chunk)
                print(f"TX audio: {len(chunk)} bytes")
        await socket.send(json.dumps({"type": "end"}))
        print("TX end")

        audio_file = None
        try:
            while True:
                message = await socket.recv()
                if isinstance(message, bytes):
                    if audio_file is None:
                        raise RuntimeError("Received audio bytes before audio_start")
                    audio_file.write(message)
                    continue

                event = json.loads(message)
                event_type = event.get("type")
                print(f"RX {event}")
                if event_type == "audio_start":
                    audio_file = output_path.open("wb")
                elif event_type == "audio_end":
                    if audio_file is not None:
                        audio_file.close()
                        audio_file = None
                        print(f"Saved TTS audio: {output_path}")
                elif event_type in {"done", "error"}:
                    if event_type == "error":
                        raise RuntimeError(event.get("message", "server error"))
                    break
        finally:
            if audio_file is not None:
                audio_file.close()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", default="ws://127.0.0.1:8088/ws/voice")
    parser.add_argument("--audio", default="speech.wav")
    parser.add_argument("--output", default="reply.wav")
    parser.add_argument("--language", default="zh")
    parser.add_argument("--chunk-size", type=int, default=4096)
    args = parser.parse_args()
    asyncio.run(run(args))


if __name__ == "__main__":
    main()
