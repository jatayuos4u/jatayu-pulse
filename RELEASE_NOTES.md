# Jatayu Pulse v1.0.0 Stable

The first stable Jatayu Pulse release is a native, read-only system monitor for **JatayuOS Udaan 1.0**, **Debian 13 (Trixie)**, and **KDE Plasma 6**.

## Highlights

- Real-time CPU and RAM metrics and history graphs
- Process monitoring
- Thermal sensors and hardware-health indications
- Storage and partition overview, including mounted/unmounted partitions
- Network activity and interface monitoring
- System information and persistent settings
- JatayuOS-native desktop launcher and application icon

## Installation (AMD64)

Download `jatayu-system-monitor_1.0.0-1_amd64.deb` from the GitHub release and run:

```bash
sudo apt install ./jatayu-system-monitor_1.0.0-1_amd64.deb
```

## Verification

The release workflow builds on Debian 13, runs the four included tests, validates package metadata, generates SHA256 checksums, and attaches the installable `.deb` to the GitHub Release.

Hardware-specific thermal and SMART readings may be unavailable depending on device configuration and permissions.

**Maintainer:** Anup Kumar Yadav <contact@jatayuos.in>  
**Website:** https://jatayuos.in  
**License:** GPL-3.0-or-later
