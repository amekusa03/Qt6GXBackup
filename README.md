# GXBackup

**English** | [日本語](README_ja.md)

**GXBackup** is a high-performance GUI backup application for Linux built with Qt6/Qt5 and C++17.  
Powered by `rsync` under the hood, it features efficient hardlink-based snapshot backups, smart auto-pausing based on system load, scheduled backup tasks, and startup catch-up execution.

![GXBackup Icon](resources/icons/gxbackup.png)

---

## 🌟 Features

- 📂 **Profile Management & Presets**
  - Store source paths, target paths, and exclude rules independently per profile.
  - One-click presets for common setups like "User Data" and "System Configuration".

- 📸 **Time Machine Style Snapshots**
  - Leverages `rsync`'s `--link-dest` option to share unchanged files via hardlinks, saving disk space while allowing full point-in-time state recovery.

- ⚡ **Smart Auto-Pause**
  - Real-time CPU usage and Load Average monitoring via `SystemMonitor`.
  - Automatically pauses backup tasks when system load exceeds thresholds (e.g. during heavy builds or encoding) and seamlessly resumes when the system cools down.

- ⏰ **Scheduling & Catch-Up Execution**
  - Configure daily or weekly automated backups at designated times.
  - Automatically detects missed backup schedules (e.g., if the PC was powered off) and triggers a "catch-up" backup upon next system boot.

- 🔔 **System Tray Residency & Autostart**
  - Minimizes to the Linux system tray, operating quietly in the background with minimal resource consumption.
  - Integrated XDG Desktop Autostart management.

- 📊 **Real-time Logging & History**
  - Live progress display showing transfer speed, completion percentage, transferred data size, and estimated time remaining.
  - Centralized execution history log dialog for tracking past backup runs.

---

## 📋 Requirements

### Dependencies
- **OS**: Linux (Ubuntu, Debian, Fedora, Arch Linux, etc.)
- **Backend Tool**: `rsync`
- **Build Tools**: CMake (3.16+), C++17 compliant compiler (GCC 8+ / Clang 7+)
- **Framework**: Qt 5 or Qt 6 (`Core`, `Gui`, `Widgets`, `Concurrent`)

#### Installing Dependencies (Debian / Ubuntu)
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev rsync
```

---

## 🛠 Building & Running

### 1. Clone the Repository
```bash
git clone https://github.com/amekusa03/GXBackup.git
cd GXBackup
```

### 2. Build
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. Run
```bash
./GXBackup
```

> **Background Mode (Minimized):**  
> Launching with `./GXBackup --minimized` starts the application directly minimized to the system tray.

---

## 📁 Project Structure

```text
GXBackup/
├── CMakeLists.txt         # CMake build configuration
├── resources/             # Icons and Qt resource definitions (.qrc)
└── src/
    ├── main.cpp           # Application entry point
    ├── core/              # Backend core engine
    │   ├── AutostartManager.cpp / .h  # Autostart configuration (.desktop)
    │   ├── BackupController.cpp / .h  # Central controller for backup workflows
    │   ├── HistoryManager.cpp / .h   # Backup execution history logger
    │   ├── ProfileManager.cpp / .h   # Profile persistence manager
    │   ├── RsyncProcess.cpp / .h     # rsync process controller & progress parser
    │   ├── ScheduleManager.cpp / .h  # Timer scheduler & catch-up detector
    │   └── SystemMonitor.cpp / .h    # CPU & Load Average monitor
    └── ui/                # Qt GUI components
        ├── HistoryDialog.cpp / .h    # History viewer dialog
        ├── LogViewer.cpp / .h        # Real-time log viewer window
        ├── MainWindow.cpp / .h       # Main application window & tray icon
        └── ProfileDialog.cpp / .h    # Profile creation & editing dialog
```

---

## 📝 Usage Guide

1. **Creating a Profile**
   - Click the "New Profile" button in the upper left.
   - Select your source and target directories.
   - Set schedule and load pause thresholds as needed, then click "Save".
2. **Starting a Backup**
   - Select a profile from the list and click "Start".
   - Monitor real-time progress and logs in the application.
3. **Checking History**
   - Click "History" to review logs and status of previous backup runs.

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
