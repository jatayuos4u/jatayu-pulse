# Jatayu Pulse v0.6.0 development source

Based on v0.5.2. Fixes stacked-page navigation order and implements Storage & Disks, Network, System Info, Settings and About pages. Storage uses Qt mounted volumes, network uses Qt network interfaces, and settings changes the refresh timer. All operations are read-only. SMART remains unavailable until a separately reviewed authorized integration is implemented.

## Build on JatayuOS / Debian 13

```bash
sudo apt install cmake ninja-build g++ qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev dpkg-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

To package after successful tests: `bash packaging/build-deb.sh`. This is development source, not a tested release binary.
