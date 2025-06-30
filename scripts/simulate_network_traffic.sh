#!/bin/bash
# Reliable Traffic Generator

set -euo pipefail

DURATION=60
SIZE=1400
BASE_IP="192.168.100"
BASE_PORT=5201
NUM_INTERFACES=4

# Handle arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --duration)
            DURATION="$2"
            shift 2
            ;;
        --size)
            SIZE="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

# Cleanup function
cleanup() {
    pkill -P $$ || true  # Kill all child processes
    echo "Traffic generation stopped"
}

trap cleanup EXIT INT TERM

echo "Starting traffic generation for $DURATION seconds..."

# Generate traffic
for i in $(seq 0 $((NUM_INTERFACES-1))); do
    ip="$BASE_IP.$((10+i))"
    port=$((BASE_PORT + i))
    
    # iperf3 traffic
    iperf3 -c "$ip" -p "$port" -t "$DURATION" -b 10M >/dev/null 2>&1 &
    echo "iperf3: host → $ip:$port (10Mbps)"
    
    # ICMP traffic
    ping "$ip" -s "$SIZE" -i 0.2 -w "$DURATION" >/dev/null &
    echo "ICMP: host → $ip (size=${SIZE}B)"
done

# Wait for completion
wait
echo "Traffic generation complete"