#!/bin/bash
# Fixed NAT Bridge Setup

set -euo pipefail

show_help() {
  cat <<EOF
Usage: sudo $0 <uplink-iface>

Creates a Linux bridge 'br-jetson' on 192.168.100.254/24 and
sets up NAT (MASQUERADE) out via <uplink-iface>.

Example:
  sudo $0 wlp0s20f3
EOF
  exit 0
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  show_help
fi

if [[ $EUID -ne 0 ]]; then
  echo "Run as root" >&2
  exit 1
fi

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <uplink-iface>" >&2
  exit 1
fi

WIFI_IF="$1"
BR="br-jetson"
SUBNET="192.168.100.0/24"
GW="192.168.100.254"

# Clean up old configuration
ip link del "$BR" 2>/dev/null || true
iptables -t nat -D POSTROUTING -s "$SUBNET" -o "$WIFI_IF" -j MASQUERADE 2>/dev/null || true

# Create bridge
ip link add name "$BR" type bridge
ip addr add "$GW/24" dev "$BR"
ip link set dev "$BR" up

# Enable NAT
sysctl -w net.ipv4.ip_forward=1 >/dev/null
iptables -t nat -A POSTROUTING -s "$SUBNET" -o "$WIFI_IF" -j MASQUERADE

# Start DHCP server
echo "Starting DHCP server"
dnsmasq \
  --interface="$BR" \
  --bind-interfaces \
  --dhcp-range=192.168.100.10,192.168.100.100,12h \
  --pid-file="/tmp/dnsmasq-$BR.pid" \
  --no-daemon >/dev/null 2>&1 &

echo "Bridge $BR up at $GW/24; NAT to $WIFI_IF"