
### Additional Helper Script

**setup_dependencies.sh**
```bash
#!/bin/bash
# setup_dependencies.sh — Install required packages

set -euo pipefail

echo "Installing system dependencies..."
sudo apt update
sudo apt install -y \
    qemu-system-arm \
    iperf3 \
    lbzip2 \
    iftop \
    iproute2 \
    bridge-utils \
    socat \
    sshpass \
    xterm

echo "Generating SSH keys..."
mkdir -p keys
ssh-keygen -t rsa -b 4096 -f keys/id_rsa -N "" -q

echo "Dependencies installed successfully"
