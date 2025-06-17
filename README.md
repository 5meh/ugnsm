# UGNSM Project

## Overview

This project provides a **Qt 6.8** application written in **C++17**, along with shell scripts to simulate and test network interfaces using QEMU and Linux TAP bridges. It consists of:

1. **Qt Application (**\`\`**)**
   - Displays network interface statistics via a grid of `NetworkInfoViewWidget` widgets.
   - Organized into subdirectories: `Core`, `UI`, `Utilities`, and `Resources`.
2. **Test Scripts**
   - `qemu-network-tester.sh`: Builds a TAP/bridge environment and launches a QEMU‑emulated Jetson Nano.
   - `simulate_network_traffic.sh`: Generates ICMP/UDP traffic and uses `tc netem` to simulate satellite link characteristics.
   - `cleanup-network.sh`: Cleans up all virtual interfaces, QEMU processes, and traffic controls.

---

## Qt Build Instructions

### Prerequisites

- **Qt 6.8** development packages
- **CMake** ≥ 3.16
- **C++ compiler** with C++17 support (GCC 9+, Clang 10+, MSVC 2019+)
- `qmake`, `cmake`, and `make` or `ninja`

### Directory Structure

```
/ (project root)
├── CMakeLists.txt         # Main project file
├── Core/                  # Core library (network logic)
├── UI/                    # UI library (widgets, UI logic)
├── Utilities/             # Utility library (logging, parsing, delegates)
├── Resources/             # Qt resource (.qrc) files and assets
├── main.cpp               # Application entrypoint
├── mainwindow.h/.cpp      # Main window class
├── mainwindow.ui          # Qt Designer UI file
└── scripts/               # Shell scripts for testing network environment
    ├── qemu-network-tester.sh
    ├── simulate_network_traffic.sh
    └── cleanup-network.sh
```

### Build Steps

1. **Create a build directory** (out‑of‑source build):

   ```bash
   mkdir -p build && cd build
   ```

2. **Configure with CMake**:

   ```bash
   cmake .. \
       -DCMAKE_PREFIX_PATH=/path/to/Qt6.8 \
       -DCMAKE_BUILD_TYPE=Release
   ```

   - `CMAKE_PREFIX_PATH`: Where Qt6.8 is installed (e.g. `/opt/Qt/6.8.0/gcc_64`).
   - `CMAKE_BUILD_TYPE`: `Release` or `Debug`.

3. **Build the project**:

   ```bash
   cmake --build . --parallel
   ```

   This compiles and links `ugnsm` with the following targets:

   - `CoreLibrary`
   - `UILibrary`
   - `UtilitiesLibrary`
   - `ResourceLibrary`

4. **Install (optional)**:

   ```bash
   cmake --install . --prefix /usr/local
   ```

   Installs the `ugnsm` binary to `/usr/local/bin` (or the configured `CMAKE_INSTALL_BINDIR`).

### Running the Application

```bash
# From build directory:
./ugnsm
# Or if installed:
ugnsm
```

---

## Script Dependencies

Before running the test scripts, ensure the following tools and kernel features are installed on your system:

- **QEMU** with AArch64 support (`qemu-system-aarch64`)
- **iproute2** utilities (`ip`, `tc`, `ip tuntap`) for managing bridges and TAP interfaces
- **socat** for UDP traffic generation
- **ping** (from `iputils`) for ICMP traffic
- **bash** (GNU Bash) for the script interpreter
- **ebtables** (optional) if your cleanup script flushes Ethernet bridge tables
- **iperf3** (optional) if you plan to benchmark with the cleanup script

And, of course, you must run the scripts as root (or via `sudo`) so they can create and configure network devices.

## Testing Network Environment

All scripts must be run as **root** (or via `sudo`) since they create TAP interfaces and apply traffic control.

### 1. Setup & Launch QEMU Environment

```bash
cd scripts
sudo ./qemu-network-tester.sh
```

- **Creates** a Linux bridge `br-jetson` and TAP interfaces `tap0`–`tap3`.
- **Starts** a QEMU aarch64 VM simulating a Jetson Nano with 4 virtio‑net devices.
- **Spawns** background ping/UDP traffic to `192.168.100.1:55555`.
- **Usage**: Press `Ctrl+C` in the script’s terminal (or kill its PID) to tear down.

### 2. Simulate Satellite Traffic

```bash
# Default parameters:
sudo ./simulate_network_traffic.sh

# Custom parameters:
sudo ./simulate_network_traffic.sh \
  --interfaces tap0,tap1 \
  --rates 10,50 \
  --latencies 150,100 \
  --jitters 30,20 \
  --duration 120
```

- **Respects** matched array lengths of interfaces, rates, latencies, and jitters.
- **Uses** `tc qdisc netem` on each interface to apply delay, jitter, and rate limits.
- **Generates** simultaneous ICMP and UDP burst traffic.
- **Stops** after `DURATION` seconds or on `Ctrl+C` (signals caught for cleanup).

### 3. Cleanup Everything

```bash
sudo ./cleanup-network.sh
```

- Kills any running `qemu-system-aarch`, traffic generator, and `iperf3` processes.
- Removes TAP interfaces (`tap0`–`tap3`, `br-jetson`, VLANs) and resets `tc qdisc`.
- Flushes `ebtables` rules applied by scripts.

---

## Detailed Script Analysis

- \`\`:

  1. \`\`: Ensures proper teardown on exit/trap.
  2. \`\`:
     - Creates bridge `br-jetson` with IP `192.168.100.1/24`.
     - For each TAP (`tap0`–`tap3`): assigns a static MAC (`52:54:00:12:34:56`–`59`), enables promiscuous mode, and enslaves to the bridge.
  3. \`\`: Launches QEMU with:
     - `virt` machine, `cortex-a57` CPU, 4 cores, 4 GB RAM.
     - Tegra DTB and rootfs image for Jetson Nano.
     - Four `virtio-net-pci` NICs linked to TAP devices.
     - Background/demo traffic loop with `ping` and `socat`.

- \`\`:

  1. **Argument parsing** for interfaces, rates, latencies, jitters, duration, packet size.
  2. \`\`: Adds missing TAPs and brings them up.
  3. \`\`: Converts Mbps → Bps → packets/sec.
  4. \`\`:
     - Applies `tc qdisc add dev <iface> root netem delay Xms Yms rate Zmbit`.
     - Spawns continuous `ping` and `socat` UDP broadcasts.
  5. **Main loop**: Iterates each interface, sets up TC + traffic, sleeps or waits based on `DURATION`.
  6. \`\`: Removes `tc` rules and kills child processes on exit.

- \`\`:

  - Kills QEMU & traffic processes via `pkill`.
  - Deletes TAPs, bridges, VLANs, resets TC qdiscs and `ebtables`.

---

With these instructions and explanations, you can **build**, **run**, and **test** the UGNSM application and its network simulation environment end‑to‑end.

