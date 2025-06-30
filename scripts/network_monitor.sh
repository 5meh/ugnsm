#!/bin/bash
# Network Interface Monitor with Full Features

set -euo pipefail

DEFAULT_INTERFACES=("tap0" "tap1" "tap2" "tap3")

show_help() {
  cat <<EOF
Usage: sudo $0 [interface1] [interface2] ...

Monitors network traffic on specified interfaces using iftop.
Defaults to monitoring ${DEFAULT_INTERFACES[@]} if no interfaces specified.

Options:
  -h, --help    Show this help message

Example:
  sudo $0 tap0 tap1  # Monitor specific interfaces
  sudo $0            # Monitor default interfaces
EOF
  exit 0
}

[[ "${1:-}" == "-h" || "${1:-}" == "--help" ]] && show_help

if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root" >&2
  exit 1
fi

# Use defaults if no interfaces specified
if [[ $# -eq 0 ]]; then
  INTERFACES=("${DEFAULT_INTERFACES[@]}")
  echo "No interfaces specified. Using defaults: ${INTERFACES[*]}"
else
  INTERFACES=("$@")
fi

# Install iftop if needed
if ! command -v iftop &> /dev/null; then
  echo "Installing iftop..."
  apt-get update && apt-get install -y iftop >/dev/null
fi

# Start monitoring each interface
for iface in "${INTERFACES[@]}"; do
  if ! ip link show "$iface" &> /dev/null; then
    echo "Interface $iface not found - skipping"
    continue
  fi
  
  echo "===================================================================="
  echo "Monitoring $iface - Press CTRL+C to exit"
  echo "===================================================================="
  iftop -i "$iface" -N -n
done