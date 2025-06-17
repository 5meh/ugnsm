#!/bin/bash
# Jetson Nano Network Test Script
# Usage: sudo ./qemu_network_tester.sh

NUM_INTERFACES=4
BRIDGE="br-jetson"
TAP_PREFIX="tap"

cleanup() {
    echo "Cleaning up network interfaces..."
    for i in $(seq 0 $((NUM_INTERFACES-1))); do
        ip link set "${TAP_PREFIX}$i" down 2>/dev/null
        ip link del "${TAP_PREFIX}$i" 2>/dev/null
    done
    ip link set $BRIDGE down 2>/dev/null
    ip link del $BRIDGE 2>/dev/null
    exit 0
}

trap cleanup INT TERM EXIT

create_network() {
    # Create bridge
    ip link add name $BRIDGE type bridge
    ip link set $BRIDGE up

    # Create TAP interfaces with MAC addresses
    for i in $(seq 0 $((NUM_INTERFACES-1))); do
        tap="${TAP_PREFIX}$i"
        ip tuntap add dev $tap mode tap
        
        # Assign MAC address (matches QEMU MACs)
        mac="52:54:00:12:34:5$((i+6))"  # 56,57,58,59
        ip link set dev $tap address $mac
        
        ip link set dev $tap up promisc on
        ip link set $tap master $BRIDGE
        echo "Created interface $tap with MAC $mac"
    done

    ip addr add 192.168.100.1/24 dev $BRIDGE
}

start_qemu() {
    qemu-system-aarch64 \
        -M virt \
        -cpu cortex-a57 \
        -smp 4 \
        -m 4G \
        -kernel kernel/Image \
        -dtb kernel/dtb/tegra210-p3448-0000-p3449-0000-a02.dtb \
        -drive file=rootfs/sd-blob-b01.img,format=raw,if=virtio,index=0 \
        -device virtio-net-pci,netdev=net0,mac=52:54:00:12:34:56 \
        -device virtio-net-pci,netdev=net1,mac=52:54:00:12:34:57 \
        -device virtio-net-pci,netdev=net2,mac=52:54:00:12:34:58 \
        -device virtio-net-pci,netdev=net3,mac=52:54:00:12:34:59 \
        -netdev tap,id=net0,ifname=tap0,script=no,downscript=no \
        -netdev tap,id=net1,ifname=tap1,script=no,downscript=no \
        -netdev tap,id=net2,ifname=tap2,script=no,downscript=no \
        -netdev tap,id=net3,ifname=tap3,script=no,downscript=no \
        -append "root=/dev/vda1 rw console=ttyAMA0" \
        -display none -daemonize
}

echo "Creating network environment..."
create_network

echo "Starting QEMU Jetson Nano emulator..."
start_qemu

echo "QEMU is running in background"
echo "Press Ctrl+C to stop testing"

# Generate some background traffic
while true; do
    for i in $(seq 0 $((NUM_INTERFACES-1))); do
        ping -I tap$i 192.168.100.1 -c 2 >/dev/null
        echo "Traffic $(date)" | socat - UDP4-DATAGRAM:192.168.100.1:55555,broadcast
    done
    sleep 1
done &
TRAFFIC_PID=$!

wait