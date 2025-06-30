#!/bin/bash
# run_network_test.sh — Orchestrate full network test
#
# Usage: sudo ./run_network_test.sh [options] [uplink-iface] [scenario]

set -euo pipefail

# ─── Defaults ────────────────────────────────────────────────────────────────
DEFAULT_UPLINK="wlp0s20f3"
DEFAULT_SCENARIO="speed"
DEFAULT_TAPS=4
GUEST_IP="192.168.100.10"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"
LOGFILE="$LOG_DIR/full_test.log"

# ─── Help ────────────────────────────────────────────────────────────────────
usage(){
  cat <<EOF
Network Test Orchestrator - UGNSM Project

Usage: sudo $0 [options] [uplink-iface] [scenario]

Options:
  -h, --help         Show this help
  -k, --keep         Keep networks after test (skip cleanup)
  -l, --logs         Also tee output to logs/full_test.log
  -n, --taps N       Number of tap interfaces (default $DEFAULT_TAPS)

Arguments:
  uplink-iface       Interface for NAT (default $DEFAULT_UPLINK)
  scenario           speed|flap|jitter|all (default $DEFAULT_SCENARIO)
EOF
  exit 0
}

# ─── Parse opts ───────────────────────────────────────────────────────────────
KEEP=0; LOGFLAG=0; NUM_TAPS=$DEFAULT_TAPS
while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help) usage ;;
    -k|--keep) KEEP=1; shift ;;
    -l|--logs) LOGFLAG=1; shift ;;
    -n|--taps) NUM_TAPS="$2"; shift 2 ;;
    --) shift; break ;;
    -* ) echo "Unknown option: $1" >&2; usage ;;
    * ) break ;;
  esac
done

UPLINK_IF="${1:-$DEFAULT_UPLINK}"
SCENARIO="${2:-$DEFAULT_SCENARIO}"

# ─── Logging ────────────────────────────────────────────────────────────────
if [[ $LOGFLAG -eq 1 ]]; then
  exec > >(tee -a "$LOGFILE") 2>&1
else
  exec > "$LOGFILE" 2>&1
fi

echo "=== NETWORK TEST START $(date) ==="
echo "Uplink: $UPLINK_IF   Scenario: $SCENARIO   Taps: $NUM_TAPS"
[[ $LOGFLAG -eq 1 ]] && echo "Logging to $LOGFILE"

# ─── Cleanup fn & trap ───────────────────────────────────────────────────────
cleanup(){
  if [[ $KEEP -eq 0 ]]; then
    echo ">>> CLEANUP: tearing down networks"
    "$SCRIPT_DIR/cleanup_networks.sh" "$UPLINK_IF"
    pkill -f qemu-system-aarch64 || true
  else
    echo ">>> PRESERVE: networks left intact (--keep)"
  fi
}
trap 'echo "*** INTERRUPTED ***"; cleanup; exit 1' INT TERM

# ─── STEP 1: initial cleanup ─────────────────────────────────────────────────
echo "--- STEP 1: cleanup ---"
cleanup || true

# ─── STEP 2: NAT bridge ─────────────────────────────────────────────────────
echo "--- STEP 2: setup NAT bridge ---"
"$SCRIPT_DIR/setup_nat_bridge.sh" "$UPLINK_IF"

# ─── STEP 3: prepare guest image ─────────────────────────────────────────────
echo "--- STEP 3: prepare guest image ---"
"$SCRIPT_DIR/prepare_guest_image.sh"
echo "✔ Guest image ready"

# ─── STEP 4: start QEMU ──────────────────────────────────────────────────────
echo "--- STEP 4: start QEMU with $NUM_TAPS taps ---"
"$SCRIPT_DIR/qemu_network_tester.sh" "$NUM_TAPS" &
QEMU_PID=$!
echo "QEMU launcher PID: $QEMU_PID"

# ─── STEP 4.5: fixed wait for boot ────────────────────────────────────────────
BOOT_WAIT=30
echo "Waiting $BOOT_WAIT s for VM to boot and network to appear..."
sleep $BOOT_WAIT

# ─── STEP 5: check ICMP before SSH ────────────────────────────────────────────
if ping -c1 -W1 "$GUEST_IP" &>/dev/null; then
  echo "✔ Guest ($GUEST_IP) answered ping"
else
  echo "⚠️  Guest did NOT answer ping—skipping SSH steps"
  cleanup
  exit 2
fi

# ─── STEP 6: setup iPerf in guest ─────────────────────────────────────────────
echo "--- STEP 6: setup iPerf servers in guest ---"
"$SCRIPT_DIR/setup_guest_iperf.sh"

# ─── STEP 7: run traffic scenario ────────────────────────────────────────────
echo "--- STEP 7: run scenario ($SCENARIO) ---"
case "$SCENARIO" in
  speed|all) \
      "$SCRIPT_DIR/simulate_network_traffic.sh" --duration 60 ;;
  flap)    "$SCRIPT_DIR/simulate_link_flap.sh"          ;;
  jitter)  "$SCRIPT_DIR/simulate_jitter.sh"            ;;
  *)       echo "Unknown scenario: $SCENARIO" >&2; cleanup; exit 1 ;;
esac

# ─── Final cleanup ────────────────────────────────────────────────────────────
cleanup
echo "=== NETWORK TEST END $(date) ==="
exit 0
