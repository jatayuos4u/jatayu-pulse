# Jatayu Pulse v1.0.1 — Security maintenance candidate

This is an **unreleased** hardening update to v1.0.0.

- Resolve `lsblk` using the trusted system path, handle timeouts, and reject oversized output
- Bound monitoring reads, process enumeration, device table rows, and thermal sensor discovery
- Correct KDE application version metadata to use the CMake project version
- Add an explicit runtime dependency on util-linux
- Document how to report vulnerabilities privately

## Local build and verification (Debian 13)

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
export JATAYU_MAINTAINER_EMAIL=contact@jatayuos.in
./packaging/build-deb.sh
```

Expected package: `jatayu-system-monitor_1.0.1-1_amd64.deb`.

Review the process list, disk partitions, temperature sensors, network view and persistence of refresh settings before publishing a stable release. Do not tag until CI and graphical regression tests pass.
