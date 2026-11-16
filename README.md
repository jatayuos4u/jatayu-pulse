# Jatayu Pulse 1.0.0 Stable

Native read-only system monitor for JatayuOS Udaan 1.0, Debian 13 (Trixie), and KDE Plasma 6.

**Maintainer:** Anup Kumar Yadav <contact@jatayuos.in>  
**Website:** https://jatayuos.in  
**License:** GPL-3.0-or-later (see COPYING and LICENSE)

## Download and install (Debian 13 / JatayuOS, AMD64)

**[Download Jatayu Pulse v1.0.0 Stable (.deb)](https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.0/jatayu-system-monitor_1.0.0-1_amd64.deb)**  
[View release notes and other downloads](https://github.com/jatayuos4u/jatayu-pulse/releases/tag/v1.0.0) · [SHA256 checksums](https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.0/SHA256SUMS)

Open a terminal and run:

```bash
wget https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.0/jatayu-system-monitor_1.0.0-1_amd64.deb
sudo apt install ./jatayu-system-monitor_1.0.0-1_amd64.deb
```

After installation, open **Jatayu Pulse** from the KDE application launcher or run:

```bash
jatayu-system-monitor
```

Verify the installed version:

```bash
dpkg-query -W -f='${Status} ${Version}\\n' jatayu-system-monitor
```

Expected: `install ok installed 1.0.0-1`.

For an existing v0.9.0 or v1.0.0 RC1 installation, the same `sudo apt install ./...` command upgrades to stable. **This package is for 64-bit x86 (amd64) systems.**

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

## Source code and releases

- [Browse complete source code](https://github.com/jatayuos4u/jatayu-pulse)
- [Official v1.0.0 stable release](https://github.com/jatayuos4u/jatayu-pulse/releases/tag/v1.0.0)
- [Release build and automated tests](https://github.com/jatayuos4u/jatayu-pulse/actions)

Do not commit the `build/` directory or `.deb` binary to Git; the release workflow publishes the Debian binary as a GitHub Release asset.
