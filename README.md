# tiBackupUi

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg?style=flat-square)](LICENSE)
[![build-check](https://github.com/theinvisible/tiBackupUi/actions/workflows/build.yml/badge.svg)](https://github.com/theinvisible/tiBackupUi/actions/workflows/build.yml)
[![Hosted By: Cloudsmith](https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square)](https://cloudsmith.com)

Graphical client of **tiBackup** — an intelligent, disk-based backup system for
Linux desktops and servers. Plug in a USB disk and a predefined backup job runs
automatically, or schedule jobs daily/weekly/monthly — with rsync, optional LUKS
encryption, pre/post scripts, e-mail notifications and Proxmox Backup Server
integration.

![tiBackupUi](https://hadler.me/wordpress/wp-content/uploads/2014/08/tibackup1.png "tiBackupUi")

> **This repository** contains `tiBackupUi`, the Qt6 Widgets **GUI** for configuring
> backup jobs, preferences, PBS servers and scripts. It runs **unprivileged** and
> performs all privileged actions through the daemon over IPC. See
> [Architecture](#architecture).

## Architecture

tiBackup consists of three parts:

| Component | Role |
|-----------|------|
| **[tiBackupLib](https://github.com/theinvisible/tiBackupLib)** | Shared core library (config, IPC, device handling, backup engine). |
| **[tiBackup](https://github.com/theinvisible/tiBackup)** | Background daemon (`tibackupd`), runs as **root**, performs the actual backups and exposes an IPC + HTTP status API. |
| **[tiBackupUi](https://github.com/theinvisible/tiBackupUi)** | Qt Widgets GUI to configure jobs and settings; runs **unprivileged** and talks to the daemon over IPC. |

The GUI never runs as root. Config writes, mounts, service control and starting
backups are relayed to the root daemon over the IPC socket; only service
start/stop is done via `systemctl` (which prompts through your polkit agent).

## Features

- Create, edit and delete **backup jobs** (devices, partitions, folders)
- **Scheduling** (daily/weekly/monthly), checksum comparison, hotplug trigger
- **LUKS encryption** and **Proxmox Backup Server (PBS)** configuration
- **Pre/post-backup script** editor
- E-mail/SMTP notification settings
- Start backups manually and watch the live status/log
- Start/stop the `tibackupd` service

## Installation

Pre-built Debian/Ubuntu packages are published via Cloudsmith:

```bash
curl -1sLf 'https://dl.cloudsmith.io/public/theinvisible/tibackup/setup.deb.sh' | sudo -E bash
sudo apt install tibackup tibackupui    # daemon + GUI
```

Enable the daemon and add yourself to the `tibackup` group so the GUI may talk to it:

```bash
sudo systemctl enable --now tibackupd
sudo usermod -aG tibackup "$USER"       # then log out and back in
```

Then launch **tiBackupUi** from your application menu (as your normal user).

## Building from source

Requirements (Debian/Ubuntu):

```bash
sudo apt install build-essential cmake qt6-base-dev \
    libpoco-dev libudev-dev libblkid-dev uuid-dev rsync cryptsetup
```

Build `tiBackupLib` first (or check it out next to this repository so it is built
automatically), then:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

> A running `tibackupd` daemon is required for full functionality; the GUI opens
> without it but shows a warning, since live device queries and backups go through
> the daemon.

## License

Licensed under the **GNU General Public License v3.0 or later** (`GPL-3.0-or-later`).
See [LICENSE](LICENSE).

## Package hosting

Package repository hosting is graciously provided by [Cloudsmith](https://cloudsmith.com).
Cloudsmith is the only fully hosted, cloud-native, universal package management
solution that lets you manage your software supply chain with confidence.
