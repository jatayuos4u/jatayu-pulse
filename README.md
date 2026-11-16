# Jatayu Pulse 1.0.0 Stable

Native read-only system monitor for JatayuOS Udaan 1.0, Debian 13 (Trixie), and KDE Plasma 6.

**Maintainer:** Anup Kumar Yadav <contact@jatayuos.in>  
**Website:** https://jatayuos.in  
**License:** GPL-3.0-or-later (see COPYING and LICENSE)

## Features

CPU and memory usage charts, processes, thermal sensors, storage/partition overview, network interfaces, system information, settings and About page. Sensors and storage health data are hardware- and permission-dependent; unavailable SMART readings are not fabricated.

## Build dependencies (Debian 13)

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev dpkg-dev libgl1-mesa-dev libxkbcommon-dev
```

## Compile and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

## Build the final Debian package

```bash
export JATAYU_MAINTAINER_EMAIL=contact@jatayuos.in
./packaging/build-deb.sh
```

The script performs a clean build and runs the four automated tests before packaging. On amd64, output is `jatayu-system-monitor_1.0.0-1_amd64.deb`.

## Upgrade and verify

```bash
sudo apt install ./jatayu-system-monitor_1.0.0-1_amd64.deb
dpkg -s jatayu-system-monitor | grep -E 'Status:|Version:'
ldd /usr/bin/jatayu-system-monitor | grep 'not found' || true
jatayu-system-monitor
```

Expected package version is `1.0.0-1`. Test the KDE launcher, icons, all pages, refresh settings and live monitoring. For an ISO release, additionally test on a fresh Debian 13 KDE Plasma 6 VM and confirm package dependencies resolve.

## GitHub

Suggested repository: `https://github.com/jatayuos4u/jatayu-pulse` (create it first).

```bash
git init
git branch -M main
git add .
git commit -m "Release Jatayu Pulse v1.0.0"
git remote add origin https://github.com/jatayuos4u/jatayu-pulse.git
git push -u origin main
git tag -a v1.0.0 -m "Jatayu Pulse 1.0.0 stable"
git push origin v1.0.0
```

Do not commit the `build/` directory or `.deb` binary to Git; attach the built `.deb` and this source archive as GitHub Release assets instead.
