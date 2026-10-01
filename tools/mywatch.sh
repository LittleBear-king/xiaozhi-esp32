#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${MYWATCH_PORT:-/dev/ttyACM0}"
IDF_PYTHON_ENV="${IDF_PYTHON_ENV:-/home/bear/.espressif/python_env/idf6.1_py3.12_env}"
IDF_EXPORT="${IDF_EXPORT:-/home/bear/.espressif/v6.1/esp-idf/export.sh}"

usage() {
    cat <<'EOF'
用法: tools/mywatch.sh <命令>

命令:
  sim       启动 UI 模拟器
  build     编译 MyWatch 固件
  flash     编译并烧录固件
  monitor   打开串口监视器
  run       编译、烧录并打开串口监视器

环境变量:
  MYWATCH_PORT=/dev/ttyACM0
  WATCH_SIMULATOR_PORT=4173
EOF
}

activate_idf() {
    cd "$PROJECT_DIR"
    # shellcheck disable=SC1090
    source "$IDF_PYTHON_ENV/bin/activate"
    # shellcheck disable=SC1090
    source "$IDF_EXPORT" >/dev/null
}

case "${1:-}" in
    sim)
        exec "$PROJECT_DIR/tools/watch-ui-simulator/start.sh" "${@:2}"
        ;;
    build)
        activate_idf
        exec python "$IDF_PATH/tools/idf.py" build
        ;;
    flash)
        activate_idf
        exec python "$IDF_PATH/tools/idf.py" -p "$PORT" flash
        ;;
    monitor)
        activate_idf
        exec python "$IDF_PATH/tools/idf.py" -p "$PORT" monitor
        ;;
    run)
        activate_idf
        exec python "$IDF_PATH/tools/idf.py" -p "$PORT" build flash monitor
        ;;
    -h|--help)
        usage
        ;;
    *)
        usage >&2
        exit 2
        ;;
esac
