# Jatayu Pulse v1.0.0

A native, read-only system monitor for **JatayuOS Udaan 1.0**, **Debian 13 (Trixie)**, and **KDE Plasma 6**, built with C++17, Qt 6, and KDE Frameworks 6.

**Maintainer:** Anup Kumar Yadav <contact@jatayuos.in>  
**Website:** https://jatayuos.in  
**License:** GPL-3.0-or-later

## Features

- Live CPU and memory charts
- Processes, temperatures, and hardware-health readings
- Storage/partition overview
- Network monitoring
- System details, settings, and About page

## Build & test (Debian 13)

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev dpkg-dev libgl1-mesa-dev libxkbcommon-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

## Build Debian package

```bash
export JATAYU_MAINTAINER_EMAIL=contact@jatayuos.in
./packaging/build-deb.sh
sudo apt install ./jatayu-system-monitor_1.0.0-1_amd64.deb
```

Download built packages from [Releases](https://github.com/jatayuos4u/jatayu-pulse/releases).

Hardware sensors and SMART availability depend on hardware, permissions, and configuration. The application does not fabricate unavailable readings.

**Repository status:** Initial metadata and automated release workflow are prepared; complete source upload is pending.
