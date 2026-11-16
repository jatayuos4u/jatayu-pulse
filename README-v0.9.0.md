# Jatayu Pulse v0.9.0 — Debian packaging beta

Based on the user-tested v0.8.0. Monitoring code is unchanged except for installed icon handling and version label.

## Build from source
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

## Build a Debian 13 .deb
Install prerequisites if necessary:
```bash
sudo apt install build-essential cmake ninja-build dpkg-dev qt6-base-dev libkf6coreaddons-dev libkf6i18n-dev
```
Supply a **real, public project contact address** (do not use a personal email unless you wish it public):
```bash
export JATAYU_MAINTAINER_EMAIL='your-project-contact@example.org'
./packaging/build-deb.sh
```
Replace the example with an actual valid address. Script builds from a temporary clean directory, runs all four tests, resolves dependencies with dpkg-shlibdeps, and creates `jatayu-system-monitor_0.9.0-1_amd64.deb` on amd64 systems.

## Inspect before installing
```bash
dpkg-deb -I jatayu-system-monitor_0.9.0-1_amd64.deb
dpkg-deb -c jatayu-system-monitor_0.9.0-1_amd64.deb
sudo apt install ./jatayu-system-monitor_0.9.0-1_amd64.deb
```
Open **Jatayu Pulse** from the application menu. Confirm icon, launcher, monitoring, persistence and no root requirement. To remove:
```bash
sudo apt remove jatayu-system-monitor
```

## Release gates
- Run all four tests on JatayuOS; test clean install, upgrade, uninstall and reinstall.
- Confirm desktop entry and both PNG/SVG icons are installed.
- Inspect package dependencies and copyright/license declarations.
- Verify package ownership and license attribution with all contributors before distribution.
- No public release until verified on a clean Debian 13 / JatayuOS environment.

License: GPL-3.0-or-later, see LICENSE and packaging/copyright. Full GPL v3 text is available on Debian at `/usr/share/common-licenses/GPL-3`. Creator: Anup Kumar Yadav. Website: https://jatayuos.in.
