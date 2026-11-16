# Jatayu Pulse 1.0.0 RC1 — Debian 13 / KDE Plasma 6

**Release candidate for testing, not yet a stable release.** The v0.9.0 beta was successfully installed and tested by the maintainer. This candidate requires fresh build, install and upgrade verification.

## Changes from v0.9.0 beta

- Fix both undefined `iconPath` references in sidebar and About page.
- Simplify application window title to **Jatayu Pulse**.
- Ship full GPL v3 text as `COPYING` alongside the existing SPDX notice and Debian copyright metadata.
- Set installed directories to mode 0755, documentation to 0644 and executable to 0755.
- Update version and Debian changelog to `1.0.0~rc1-1`. The tilde ensures an eventual `1.0.0-1` stable package upgrades correctly.

## Build prerequisites

On Debian 13 / JatayuOS, install CMake, Ninja, a C++ compiler, Qt6 Core/Widgets/Network/Test development packages, KDE Frameworks 6 CoreAddons and I18n development packages, and `dpkg-dev`. See `README-v0.9.0.md` for prior setup details.

## Compile and test

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

## Build .deb (clean build and tests are run again)

```bash
export JATAYU_MAINTAINER_EMAIL=contact@jatayuos.in
./packaging/build-deb.sh
```

Expected amd64 output: `jatayu-system-monitor_1.0.0~rc1-1_amd64.deb`.

## Check, upgrade, and verify

```bash
dpkg-deb --info ./jatayu-system-monitor_1.0.0~rc1-1_amd64.deb
dpkg-deb --contents ./jatayu-system-monitor_1.0.0~rc1-1_amd64.deb
sudo apt install ./jatayu-system-monitor_1.0.0~rc1-1_amd64.deb
dpkg -s jatayu-system-monitor | grep -E 'Status:|Version:'
ldd /usr/bin/jatayu-system-monitor | grep 'not found' || true
jatayu-system-monitor
```

Expect `Status: install ok installed` and `Version: 1.0.0~rc1-1`. Test all pages, icon and KDE launcher, and verify CPU, RAM, thermal, network and storage metrics. The no-output `ldd` check is good. Avoid running the graphical application as root.

## Optional uninstall test

Only if you want to test removal: `sudo apt remove jatayu-system-monitor`. Reinstall afterward using the `apt install` command above.

## Release notes

The candidate is read-only and does not require root for monitoring. Sensor availability depends on hardware and permissions. Package testing on a second clean Debian 13 KDE Plasma 6 installation is recommended before declaring v1.0.0 stable.
