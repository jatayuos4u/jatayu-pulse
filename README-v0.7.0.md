# Jatayu Pulse v0.7.0 — Functional Preview

- Process Explorer: sortable PID/name/RAM columns; complete process-name tooltips; 150 sampled processes.
- Network: live download/upload readings, reference-scale charts (100% = 100 KiB/s), interface lifetime byte counters, elapsed-time-based speeds.
- Settings: refresh interval saved with QSettings and restored at startup.
- Dashboard: retains scrolling and adds a scrolling hint.
- Tests: new network-rate tests for normal, variable interval, zero time, and counter reset.

Network graphs currently use a fixed 0–100 KiB/s reference scale. Higher rates are capped visually, but numeric speeds are not capped. This is a development preview, not a release candidate.

Build on Debian 13 with Qt 6 and KDE Frameworks 6:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```
