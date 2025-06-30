#!/bin/bash
# qemu_network_tester.sh — Launch QEMU with N TAP interfaces, then exit

set -euo pipefail

usage(){
  cat <<EOF
Usage: sudo $0 [num_taps]

Creates <num_taps> TAP interfaces on bridge 'br-jetson' and launches QEMU.
Default num_taps=4.
EOF
  exit 1
}

NUM_IF=${1:-4}
BRIDGE="br-jetson"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"&&pwd)"
IMG="$SCRIPT_DIR/rootfs/sd-blob-b01.img"
KERNEL="$SCRIPT_DIR/kernel/Image"
DTB="$SCRIPT_DIR/kernel/dtb/tegra210-p3448-0000-p3449-0000-a02.dtb"

# ensure bridge exists
if ! ip link show "$BRIDGE" &>/dev/null; then
  echo "ERROR: Bridge $BRIDGE not found. Run setup_nat_bridge.sh first." >&2
  exit 1
fi

# create TAPs
for i in $(seq 0 $((NUM_IF-1))); do
  tap="tap$i"
  ip link del "$tap" 2>/dev/null || true
  ip tuntap add dev "$tap" mode tap
  ip link set dev "$tap" master "$BRIDGE"
  ip link set dev "$tap" up promisc on
  echo "Created $tap"
done

# launch QEMU in background and return
echo "Launching QEMU (background)"
exec qemu-system-aarch64 \
  -M virt -cpu cortex-a57 -smp 4 -m 4G \
  -kernel "$KERNEL" \
  -dtb "$DTB" \
  -drive file="$IMG",format=raw,if=virtio,index=0 \
  $(for i in $(seq 0 $((NUM_IF-1))); do
      printf -- "-netdev tap,id=net%d,ifname=tap%d,script=no,downscript=no " "$i" "$i"
      printf -- "-device virtio-net-pci,netdev=net%d,mac=52:54:00:12:34:5%d " "$i" "$((6+i))"
    done) \
  -append "root=/dev/vda1 rw console=ttyAMA0" \
  -display none -daemonize
