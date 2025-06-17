#!/bin/bash
# Robust Network Traffic Simulator
# Simulates satellite link traffic for Jetson Nano testing
# Usage: sudo ./simulate_network_traffic.sh [OPTIONS]

# Satellite link characteristics
INTERFACES=("tap0" "tap1" "tap2" "tap3")
RATES_MBPS=(5 10 25 50)       # Mbps for each interface (simulating different satellite links)
LATENCIES=(100 75 50 25)      # Latency in ms for each interface
JITTERS=(20 15 10 5)          # Jitter in ms for each interface
DURATION=3600                 # Run duration in seconds (0=forever)
PACKET_SIZE=1400              # Bytes

show_help() {
    echo "Satellite Link Traffic Simulator"
    echo "Usage: sudo $0 [OPTIONS]"
    echo "Options:"
    echo "  -h, --help          Show this help"
    echo "  -i, --interfaces    Comma-separated interface list (default: tap0,tap1,tap2,tap3)"
    echo "  -r, --rates         Comma-separated rates in Mbps (default: 5,10,25,50)"
    echo "  -l, --latencies     Comma-separated latencies in ms (default: 100,75,50,25)"
    echo "  -j, --jitters       Comma-separated jitters in ms (default: 20,15,10,5)"
    echo "  -d, --duration      Duration in seconds (0=forever, default: 3600)"
    echo "  -s, --size          Packet size in bytes (default: 1400)"
    echo ""
    echo "Example:"
    echo "  sudo $0 -i tap0,tap1 -r 10,50 -l 150,100 -j 30,20 -d 120"
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -h|--help)
            show_help
            exit 0
            ;;
        -i|--interfaces)
            IFS=',' read -ra INTERFACES <<< "$2"
            shift 2
            ;;
        -r|--rates)
            IFS=',' read -ra RATES_MBPS <<< "$2"
            shift 2
            ;;
        -l|--latencies)
            IFS=',' read -ra LATENCIES <<< "$2"
            shift 2
            ;;
        -j|--jitters)
            IFS=',' read -ra JITTERS <<< "$2"
            shift 2
            ;;
        -d|--duration)
            DURATION="$2"
            shift 2
            ;;
        -s|--size)
            PACKET_SIZE="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Check root
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

# Validate input
if [[ ${#INTERFACES[@]} -ne ${#RATES_MBPS[@]} || 
      ${#INTERFACES[@]} -ne ${#LATENCIES[@]} ||
      ${#INTERFACES[@]} -ne ${#JITTERS[@]} ]]; then
    echo "Error: Interface, rate, latency and jitter counts must match"
    exit 1
fi

# Create TAP interfaces if missing
create_interfaces() {
    for iface in "${INTERFACES[@]}"; do
        if ! ip link show "$iface" &>/dev/null; then
            echo "Creating satellite link interface $iface"
            ip tuntap add dev "$iface" mode tap
            ip link set "$iface" up
        fi
    done
}

# Calculate packets per second for each interface (integer math)
calculate_pps() {
    local rate_mbps=$1
    # Convert Mbps to Bps: Mbps * 125000 (1000000/8)
    local rate_bps=$((rate_mbps * 125000))
    # Packets per second: Bps / packet_size
    echo $((rate_bps / PACKET_SIZE))
}

# Simulate traffic for a satellite link
simulate_satellite_link() {
    local iface=$1
    local rate_mbps=$2
    local latency=$3
    local jitter=$4
    
    # Calculate packets per second
    local pps=$(calculate_pps "$rate_mbps")
    
    echo "Simulating satellite link on $iface: ${rate_mbps}Mbps, ${latency}ms latency, ${jitter}ms jitter"
    
    # Apply traffic control to simulate satellite characteristics
    tc qdisc add dev $iface root netem delay ${latency}ms ${jitter}ms rate ${rate_mbps}mbit
    
    # Generate background traffic (ICMP + UDP)
    ping -I $iface 192.168.100.1 -i 0.2 -q >/dev/null &
    local ping_pid=$!
    
    while :; do
        # Generate UDP traffic
        echo "UDP Traffic $(date)" | socat - UDP4-DATAGRAM:192.168.100.1:55555,broadcast
        sleep 0.1
    done &
    local udp_pid=$!
    
    # Return PIDs for cleanup
    echo "$ping_pid $udp_pid"
}

# Main cleanup function
cleanup() {
    echo "Stopping satellite link simulation..."
    # Remove traffic control
    for iface in "${INTERFACES[@]}"; do
        tc qdisc del dev $iface root 2>/dev/null
    done
    
    # Kill background processes
    pkill -P $$  # Kill all child processes
    exit 0
}

trap cleanup INT TERM

# Create interfaces if needed
create_interfaces

# Start simulation for all interfaces
declare -A TRAFFIC_PIDS
for idx in "${!INTERFACES[@]}"; do
    iface=${INTERFACES[$idx]}
    rate=${RATES_MBPS[$idx]}
    latency=${LATENCIES[$idx]}
    jitter=${JITTERS[$idx]}
    
    # Verify interface exists
    if ! ip link show "$iface" &> /dev/null; then
        echo "Satellite link interface $iface not found! Skipping..."
        continue
    fi
    
    # Start simulation
    pids=$(simulate_satellite_link "$iface" "$rate" "$latency" "$jitter")
    TRAFFIC_PIDS["$iface"]=$pids
    echo "Started satellite simulation on $iface (PIDs: $pids)"
done

# Wait for completion or until interrupted
if [[ $DURATION -gt 0 ]]; then
    echo "Satellite link simulation running for $DURATION seconds..."
    sleep $DURATION
else
    echo "Satellite link simulation running indefinitely. Press Ctrl+C to stop."
    wait
fi

echo "Satellite link simulation completed"