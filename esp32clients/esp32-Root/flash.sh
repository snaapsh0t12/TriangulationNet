#!/usr/bin/env bash
set -euo pipefail

# One-command ESP32 flasher for this project.
# Defaults match the verified working setup in this workspace.

PORT="${ESP_PORT:-/dev/ttyUSB0}"
BAUD="${ESP_BAUD:-460800}"
IMAGE="build/esp32-Root.ino.merged.bin"
ERASE=0
MONITOR=0
MONITOR_SECONDS="${ESP_MONITOR_SECONDS:-10}"

usage() {
  cat <<'EOF'
Usage: ./flash.sh [options]

Options:
  -p, --port <port>       Serial port (default: /dev/ttyUSB0)
  -b, --baud <baud>       Flash baud (default: 460800)
  -i, --image <path>      Firmware image path (default: build/esp32-Node.ino.merged.bin)
  -e, --erase             Erase full flash before writing
  -m, --monitor           Read serial output after flashing
  -t, --time <seconds>    Monitor duration with --monitor (default: 10)
  -h, --help              Show this help

Examples:
  ./flash.sh
  ./flash.sh --erase --monitor
  ./flash.sh -p /dev/ttyUSB1 -b 921600
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -p|--port)
      PORT="$2"
      shift 2
      ;;
    -b|--baud)
      BAUD="$2"
      shift 2
      ;;
    -i|--image)
      IMAGE="$2"
      shift 2
      ;;
    -e|--erase)
      ERASE=1
      shift
      ;;
    -m|--monitor)
      MONITOR=1
      shift
      ;;
    -t|--time)
      MONITOR_SECONDS="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ ! -f "$IMAGE" ]]; then
  echo "Firmware image not found: $IMAGE" >&2
  echo "Build your sketch first so the merged image exists." >&2
  exit 1
fi

if [[ ! -c "$PORT" ]]; then
  echo "Serial device not found: $PORT" >&2
  exit 1
fi

if command -v esptool.py >/dev/null 2>&1; then
  ESPTOOL=(esptool.py)
elif command -v esptool >/dev/null 2>&1; then
  ESPTOOL=(esptool)
elif [[ -f "$HOME/.arduino15/packages/esp32/tools/esptool_py/4.6/esptool.py" ]]; then
  ESPTOOL=(python3 "$HOME/.arduino15/packages/esp32/tools/esptool_py/4.6/esptool.py")
else
  echo "Could not find esptool. Install ESP32 platform tools or add esptool.py to PATH." >&2
  exit 1
fi

echo "Using esptool: ${ESPTOOL[*]}"
echo "Port: $PORT"
echo "Image: $IMAGE"

"${ESPTOOL[@]}" --port "$PORT" chip_id

if [[ "$ERASE" -eq 1 ]]; then
  echo "Erasing flash..."
  "${ESPTOOL[@]}" --chip esp32 --port "$PORT" erase_flash
fi

echo "Writing firmware..."
"${ESPTOOL[@]}" --chip esp32 --port "$PORT" --baud "$BAUD" write_flash 0x0 "$IMAGE"

echo "Flash complete."

if [[ "$MONITOR" -eq 1 ]]; then
  echo "Reading serial output for ${MONITOR_SECONDS}s at 115200..."
  # Keep monitor short and non-interactive so script always returns.
  timeout "${MONITOR_SECONDS}s" cat "$PORT" || true
fi
