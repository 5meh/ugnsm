#!/bin/bash
# SSH Connectivity Test

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GUEST_IP="${1:?Guest IP required}"
COMMAND="${2:-true}"
SSH_USER="ubuntu"
SSH_KEY="$SCRIPT_DIR/keys/id_rsa"

ssh -o ConnectTimeout=5 \
    -o StrictHostKeyChecking=no \
    -o UserKnownHostsFile=/dev/null \
    -i "$SSH_KEY" \
    "$SSH_USER@$GUEST_IP" "$COMMAND" >/dev/null 2>&1
