#!/bin/bash
# Fixed iPerf Server Setup with SSH Reliability

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="/tmp/setup_guest_iperf.log"

GUEST_IP="192.168.100.10"
SSH_OPTS=(-o ConnectTimeout=5
          -o StrictHostKeyChecking=no
          -o UserKnownHostsFile=/dev/null
          -o LogLevel=ERROR)
SSH_USER="ubuntu"
SSH_KEY="$SCRIPT_DIR/keys/id_rsa"
BASE_PORT=5201

# Wait for SSH with more reliable check
wait_for_ssh() {
  echo -n "Waiting for guest SSH "
  for i in {1..30}; do
    if ssh "${SSH_OPTS[@]}" -i "$SSH_KEY" "$SSH_USER@$GUEST_IP" true; then
      echo " connected!"
      return 0
    fi
    echo -n "."
    sleep 1
  done
  echo " timeout!"
  return 1
}

if ! wait_for_ssh; then
  exit 1
fi

# Install iperf3
echo "Installing iperf3 in guest"
ssh $SSH_OPTS -i "$SSH_KEY" $SSH_USER@$GUEST_IP \
  "sudo apt-get update && sudo apt-get install -y iperf3" &>> "$LOG_FILE"

# Start iperf servers directly (no systemd)
for i in {0..3}; do
  port=$((BASE_PORT + i))
  ip="192.168.100.$((10+i))"
  
  echo "Starting iperf3 server on $ip:$port"
  ssh $SSH_OPTS -i "$SSH_KEY" $SSH_USER@$GUEST_IP \
    "sudo pkill -f 'iperf3 -s.*:$port' || true"
  ssh $SSH_OPTS -i "$SSH_KEY" $SSH_USER@$GUEST_IP \
    "sudo iperf3 -s -B $ip -p $port -D" &>> "$LOG_FILE"
done

echo "iperf3 servers active on ports $BASE_PORT-$((BASE_PORT+3))"