#!/bin/bash
# prepare_guest_image.sh — Guest OS preparation, now static IP

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="${SCRIPT_DIR}/rootfs/sd-blob-b01.img"
KEY_PUB="${SCRIPT_DIR}/keys/id_rsa.pub"
MOUNT_DIR="/mnt/guest"

# cleanup old mounts/loops
umount "$MOUNT_DIR" 2>/dev/null || true
losetup -D 2>/dev/null || true

# verify
[[ -f "$IMAGE" ]]   || { echo "Missing image: $IMAGE" >&2; exit 1; }
[[ -f "$KEY_PUB" ]] || { echo "Missing SSH key: $KEY_PUB" >&2; exit 1; }

mkdir -p "$MOUNT_DIR"

# map loop with partitions
LOOP=$(losetup --find --show -P "$IMAGE")
mount "${LOOP}p1" "$MOUNT_DIR"

# SSH key
mkdir -p "$MOUNT_DIR/home/ubuntu/.ssh"
cp "$KEY_PUB" "$MOUNT_DIR/home/ubuntu/.ssh/authorized_keys"
chown -R 1000:1000 "$MOUNT_DIR/home/ubuntu/.ssh"

# static netplan on eth0
cat > "$MOUNT_DIR/etc/netplan/01-static.yaml" <<EOF
network:
  version: 2
  renderer: networkd
  ethernets:
    eth0:
      addresses: [192.168.100.10/24]
      gateway4: 192.168.100.254
      nameservers:
        addresses: [8.8.8.8,8.8.4.4]
EOF

# cleanup
umount "$MOUNT_DIR"
losetup -d "$LOOP"

echo "✔ Guest image configured with static IP 192.168.100.10"
