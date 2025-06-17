#!/bin/bash
# Network Cleanup Script
# Usage: sudo ./cleanup-network.sh

echo "Cleaning up network resources..."

# Kill QEMU and traffic processes
pkill -f qemu-system-aarch
pkill -f simulate_network_traffic
pkill iperf3

# Remove network interfaces
for i in {0..3}; do
    ip link set "tap$i" down 2>/dev/null
    ip link del "tap$i" 2>/dev/null
    ip link set "br$i" down 2>/dev/null
    ip link del "br$i" 2>/dev/null
done

# Remove main bridge
ip link set br-jetson down 2>/dev/null
ip link del br-jetson 2>/dev/null

# Remove VLAN interface
ip link set br-jetson.100 down 2>/dev/null
ip link del br-jetson.100 2>/dev/null

# Reset traffic control
tc qdisc del dev br-jetson root 2>/dev/null

# Reset ebtables
ebtables -F 2>/dev/null

echo "Network cleanup complete"