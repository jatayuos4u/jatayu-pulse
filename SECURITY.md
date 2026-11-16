# Security policy

## Supported versions

Security fixes are prioritized for the latest stable Jatayu Pulse release.

## Reporting a vulnerability

Do not open a public GitHub issue with exploit details, passwords, tokens, or private system data.

Email **contact@jatayuos.in** with affected version, steps to reproduce, impact, and sanitized logs. We will triage reports and coordinate remediation and disclosure.

Maintainers should enable **Private vulnerability reporting** in GitHub Settings → Security.

## Security model

Jatayu Pulse is intended to run without root privileges. It reads Linux procfs/sysfs information and launches the system `/usr/bin/lsblk` to list storage. Hardware-derived inputs and environment values may be malformed or unavailable and must be handled defensively. Do not run Jatayu Pulse as root.
