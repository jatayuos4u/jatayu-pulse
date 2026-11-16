# Jatayu Pulse 0.7.1 — Network Monitoring Refinement

Changes from v0.7.0:
- Download/upload charts now use adaptive vertical axes with KiB/s or MiB/s labels. The maximum is recalculated from the last 60 samples using 1/2/5/10 scale steps.
- Network interface cumulative traffic counters show B, KiB, MiB or GiB rather than rounding small transfers to 0.0 GiB.
- Missing or inaccessible network counters display `Unavailable`.
- Existing read-only process explorer and saved refresh interval behavior retained.

Build and test on Debian 13 / JatayuOS:
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

Notes: Traffic graphs are based on total interface counters, and include loopback/virtual interfaces as in v0.7.0. Adaptive axes may change as old spikes leave the 60-sample window. This source was not compiled in the authoring environment; test on JatayuOS before packaging.
