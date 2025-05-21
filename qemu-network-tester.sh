#!/bin/bash
# QEMU Jetson Nano Network Test Script
# Usage: sudo ./qemu-network-tester.sh [SCENARIO]

SCENARIO=${1:-basic}
NUM_INTERFACES=4
BRIDGE="br-jetson"
TAP_PREFIX="tap"

cleanup() {
    echo "Cleaning up network interfaces..."
    for i in $(seq 0 $((NUM_INTERFACES-1))); do
        ip link del "${TAP_PREFIX}$i" 2>/dev/null
    done
    ip link del $BRIDGE 2>/dev/null
    tc qdisc del dev $BRIDGE root 2>/dev/null
}

create_base_network() {
    # Create bridge
    ip link add name $BRIDGE type bridge
    ip link set $BRIDGE up

    # Create TAP interfaces
    for i in $(seq 0 $((NUM_INTERFACES-1))); do
        tap="${TAP_PREFIX}$i"
        ip tuntap add dev $tap mode tap
        ip link set dev $tap up promisc on
        ip link set $tap master $BRIDGE
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
        -display gtk
}

case $SCENARIO in
    basic)
        echo "Running BASIC network scenario"
        create_base_network
        ;;
        
    vlan)
        echo "Running VLAN scenario"
        create_base_network
        # Create VLAN 100 on bridge
        ip link add link $BRIDGE name ${BRIDGE}.100 type vlan id 100
        ip link set ${BRIDGE}.100 up
        ip addr add 192.168.100.100/24 dev ${BRIDGE}.100
        ;;

    traffic-shaping)
        echo "Running TRAFFIC SHAPING scenario"
        create_base_network
        # Limit bridge to 100Mbps with 50ms latency
        tc qdisc add dev $BRIDGE root handle 1: htb default 1
        tc class add dev $BRIDGE parent 1: classid 1:1 htb rate 100mbit
        tc qdisc add dev $BRIDGE parent 1:1 handle 10: netem delay 50ms
        ;;

    isolated)
        echo "Running ISOLATED NETWORKS scenario"
        # Create separate bridges for each interface
        for i in $(seq 0 $((NUM_INTERFACES-1))); do
            br="br$i"
            tap="${TAP_PREFIX}$i"
            ip link add name $br type bridge
            ip link set $br up
            ip link set $tap master $br
            ip addr add 192.168.10$i.1/24 dev $br
        done
        ;;

    mac-filtering)
        echo "Running MAC FILTERING scenario"
        create_base_network
        # Allow only specific MAC addresses
        ebtables -A FORWARD -s 52:54:00:12:34:56 -j ACCEPT
        ebtables -A FORWARD -s 52:54:00:12:34:57 -j ACCEPT
        ebtables -A FORWARD -j DROP
        ;;

    *)
        echo "Available scenarios:"
        echo "  basic, vlan, traffic-shaping, isolated, mac-filtering"
        exit 1
        ;;
esac

start_qemu
cleanup
