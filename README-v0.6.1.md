# Jatayu Pulse 0.6.1 — hardware information and layout update

Changes: physical disk and partition tree using `lsblk -J -b` (read-only), including unmounted Windows NTFS partitions where visible to Linux; dashboard physical-disk capacity alongside the Linux root partition; fixed-width PID and RAM process table columns with memory descending; expanded system information (CPU, RAM, motherboard, firmware); professional About page with version, SPDX GPL-3.0-or-later license, and clickable JatayuOS / GPL links.

The sensor list includes all readings returned by the existing `Thermal::discover()` implementation under `/sys/class/hwmon`; it is not a guarantee of every physical hardware sensor. SMART, GPU-specific sensors and disk temperature require additional support. Physical drive capacity is not the same as mounted filesystem free space. Disk enumeration uses the system's `lsblk` executable; if absent, a warning row is displayed. Windows partitions are never mounted or modified.

**Build on Debian 13 / JatayuOS:**

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/jatayu-system-monitor
```

This source package has not been compiled or run in the authoring environment. Test on your JatayuOS system before release.
