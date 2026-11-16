# Jatayu Pulse v0.5.2 — responsive dashboard patch

Based on v0.5.1 source. Changes: 3×2 compact metric grid, reduced padding and chart minimum heights, readable sidebar OS branding, CPU-specific temperature label, and honest elapsed-history chart label.

## Build (Debian 13)
```sh
sudo apt install cmake ninja-build g++ qt6-base-dev libkf6i18n-dev libkf6coreaddons-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

This source has not been compiled in the packaging environment. Test on JatayuOS before distribution. Dashboard may still scroll at large font scaling.
