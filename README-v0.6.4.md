# Jatayu Pulse 0.6.4 — Performance fix

The CPU and memory summary cards now use persistent member pointers rather than unsuccessful QObject name searches. The history description no longer assumes a fixed refresh interval.

Build:
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```
