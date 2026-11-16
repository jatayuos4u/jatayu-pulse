# Jatayu Pulse 0.6.3 — Performance preview

Focused performance page redesign: three live summary tiles (CPU, memory and swap), live metrics in chart headings, improved chart hierarchy and spacing. Existing 60-sample charts and monitoring logic preserved.

Build on JatayuOS:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```
