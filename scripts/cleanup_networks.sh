#!/bin/bash
# Robust Network Cleanup with Uplink Fix

set -euo pipefail

if [[ $EUID -ne 0 ]]; then
  echo "Run as root" >&2
  exit 1
fi

# Get uplink interface from parameter or use default
UPLINK_IF="${1:-wlp0s20f3}"
BR="br-jetson"
SUBNET="192.168.100.0/24"

echo "Performing network cleanup..."

# Remove bridge
ip link del "$BR" 2>/dev/null || true

# Remove TAP interfaces
for i in {0..3}; do
  ip link del "tap$i" 2>/dev/null || true
done

# Remove NAT rules
iptables -t nat -D POSTROUTING -s "$SUBNET" -o "$UPLINK_IF" -j MASQUERADE 2>/dev/null || true

# Clean mounts
umount /mnt/guest 2>/dev/null || true
losetup -D 2>/dev/null || true

# Kill DHCP server
pkill -f "dnsmasq.*--interface=$BR" || true
rm -f "/tmp/dnsmasq-$BR.pid"

# Kill QEMU processes
pkill -f "qemu-system-aarch64.*kernel/Image" || true
pkill -f "qemu-system-aarch64" || true
pkill -f "qemu.*aarch64" || true

echo "Network cleanup complete."