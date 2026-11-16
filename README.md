# Jatayu Pulse v1.0.1 Stable

Native, read-only system monitor for **JatayuOS Udaan 1.0**, **Debian 13 (Trixie)**, and **KDE Plasma 6**.

**Maintainer:** Anup Kumar Yadav <contact@jatayuos.in>  
**Website:** https://jatayuos.in  
**License:** GPL-3.0-or-later (see [COPYING](COPYING) and [LICENSE](LICENSE))

## Download and install (Debian 13 / JatayuOS, AMD64)

**[Download Jatayu Pulse v1.0.1 Stable (.deb)](https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.1/jatayu-system-monitor_1.0.1-1_amd64.deb)**

[Official v1.0.1 release](https://github.com/jatayuos4u/jatayu-pulse/releases/tag/v1.0.1) · [SHA256 checksums](https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.1/SHA256SUMS)

Open a terminal and run:

```bash
wget https://github.com/jatayuos4u/jatayu-pulse/releases/download/v1.0.1/jatayu-system-monitor_1.0.1-1_amd64.deb
sudo apt install ./jatayu-system-monitor_1.0.1-1_amd64.deb
```

Open **Jatayu Pulse** from the KDE application launcher, or run:

```bash
jatayu-system-monitor
```

Verify your installed version:

```bash
dpkg-query -W -f='${Status} ${Version}\n' jatayu-system-monitor
```

Expected: `install ok installed 1.0.1-1`.

The same APT installation command upgrades earlier versions, including v0.9.0, v1.0.0 RC1, and v1.0.0 Stable. **This package supports x86-64 (amd64); it is not an ARM package.**

## Features

- Live CPU and memory usage charts
- Processes and system resource monitoring
- Hardware thermal sensors
- Storage devices, partitions, and mount information
- Network interface monitoring
- System details, persistent settings, and About page

Hardware sensors and storage-health data depend on the hardware and permissions; unavailable SMART readings are not fabricated. Jatayu Pulse is intended to run as a normal user, not root.

## Build dependencies (Debian 13)

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev dpkg-dev libgl1-mesa-dev libxkbcommon-dev
```

## Compile and test from source

```bash
git clone https://github.com/jatayuos4u/jatayu-pulse.git
cd jatayu-pulse
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

For a reproducible version checkout, run `git checkout v1.0.1` before building.

## Build the Debian package

```bash
export JATAYU_MAINTAINER_EMAIL=contact@jatayuos.in
./packaging/build-deb.sh
```

The packaging script performs a clean build and runs four automated tests. Expected AMD64 output: `jatayu-system-monitor_1.0.1-1_amd64.deb`.

## Upgrade and verify

```bash
sudo apt install ./jatayu-system-monitor_1.0.1-1_amd64.deb
dpkg -s jatayu-system-monitor | grep -E 'Status:|Version:'
ldd /usr/bin/jatayu-system-monitor | grep 'not found' || true
jatayu-system-monitor
```

Expected package version: `1.0.1-1`. Check the KDE launcher, icons, dashboard, disk/process/thermal pages, refresh settings, and live metrics. For JatayuOS ISO integration, also test on a fresh Debian 13 KDE Plasma 6 installation.

## Security

Please report suspected vulnerabilities privately using the instructions in [SECURITY.md](SECURITY.md). Avoid posting exploit details or sensitive system logs in public issues.

## Source code and releases

- [Latest stable release (v1.0.1)](https://github.com/jatayuos4u/jatayu-pulse/releases/tag/v1.0.1)
- [Previous releases](https://github.com/jatayuos4u/jatayu-pulse/releases)
- [Source repository](https://github.com/jatayuos4u/jatayu-pulse)
- [Automated build and test results](https://github.com/jatayuos4u/jatayu-pulse/actions)

Do not commit build directories or `.deb` binaries into Git. The tagged release workflow publishes the binary and SHA256 checksum as release assets.
