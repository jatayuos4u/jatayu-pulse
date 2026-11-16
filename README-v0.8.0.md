# Jatayu Pulse v0.8.0 — Stability Preview

Based on the user-tested v0.7.1. Preserves existing visuals and network graphs.

## Stability changes
- Uses a monotonic timer for network-rate sampling (resists wall-clock changes).
- Rejects impossible/very long sampling intervals and counter resets.
- Prevents invalid memory-available values from causing unsigned underflow.
- Guards cumulative network counters against integer overflow.
- Adds automated edge-case tests for zero RAM/swap, invalid intervals and counter resets.
- Updates Debian packaging script version to 0.8.0-1 (experimental only).

## Build
```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

## Manual test matrix
1. Test CPU, memory and swap under normal load; test a machine with no swap if available.
2. Download a large file and verify network speed and autoscaled graphs; disconnect and reconnect Wi-Fi.
3. Confirm Windows and Linux partitions appear in Storage & Disks; no write actions occur.
4. Verify CPU temperature and the no-sensor case (on hardware without readable sensors).
5. Sort/filter Process Explorer; restart and confirm Settings persistence.
6. Leave the monitor open for at least 30 minutes; check responsiveness and memory growth.
7. Run the application without root privileges.

## Packaging caution
`packaging/build-deb.sh` is an early experimental packaging helper. Inspect package control metadata and dependencies before distribution. The `anup@localhost` maintainer address is a placeholder and MUST be replaced with a real project contact before publishing. A full GPL-3.0-or-later license copy and reproducible build/upgrade tests are still required for a public release. The root README's license declaration is not a substitute for including the full GPL text in a distributed package.

This archive has not been compiled or tested on Debian 13 by the assistant. Validate on JatayuOS before calling it release-ready.
