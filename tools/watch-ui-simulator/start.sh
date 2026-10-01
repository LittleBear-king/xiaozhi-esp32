#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PORT="${WATCH_SIMULATOR_PORT:-4173}"
OPEN_BROWSER=1

while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)
            PORT="${2:?missing port}"
            shift 2
            ;;
        --no-browser)
            OPEN_BROWSER=0
            shift
            ;;
        -h|--help)
            printf '用法: %s [--port PORT] [--no-browser]\n' "$0"
            exit 0
            ;;
        *)
            printf '未知参数: %s\n' "$1" >&2
            exit 2
            ;;
    esac
done

URL="http://127.0.0.1:${PORT}/"
printf 'MyWatch UI 模拟器: %s\n' "$URL"
printf '按 Ctrl+C 停止服务\n'

if [[ "$OPEN_BROWSER" -eq 1 ]] && command -v xdg-open >/dev/null 2>&1; then
    (sleep 1; xdg-open "$URL" >/dev/null 2>&1 || true) &
fi

cd "$SCRIPT_DIR"
exec python3 -m http.server "$PORT" --bind 127.0.0.1
