# GD Mobile Stats 📱⚡

A Geode mod for **Geometry Dash** that turns your phone into an external live HUD and remote controller over a USB-C cable or local Wi-Fi — **directly in any mobile web browser!**

---

## ✨ Features
- **🌐 Built-in Web HUD (Zero APK / Zero Install Required)**:
  - Just open your phone's browser (Chrome, Safari, Firefox, etc.) to `http://localhost:34567` (USB) or `http://<PC_IP>:34567` (Wi-Fi).
  - Works on Android, iPhone, iPad, or any device!
  - Supports **Portrait AND Landscape** modes (optimized split-screen layout for landscape).
  - **Screen Wake Lock**: Phone screen stays awake automatically while playing.
  - **Haptic Vibration**: Physical click vibration feedback when tapping StartPos and Respawn buttons.
  - **Fullscreen Toggle**: Borderless immersive HUD display.
- **📊 Real-time Stats Streaming**:
  - Live decimal percentage (`45.2%`).
  - Level name & creator (defaults to RobTop on official levels).
  - Practice Mode vs Normal Mode pill.
  - Total Attempts, Session Attempts, and Session Time played.
  - Run Status: `PLAYING` (Green), `PAUSED` (Orange), `CRASHED` (Red), `COMPLETE` (Gold).
- **🕹 StartPos Remote Controller & Respawn**:
  - `◀ Prev StartPos` (Key `Q`)
  - `Next StartPos ▶` (Key `E`)
  - `↺ Quick Respawn` (Key `R`)
  - Full compatibility with MegaHack and other keybind-based mods.
- **⚡ Wired & Wireless Support**:
  - **Wired (USB-C)**: Built-in silent `adb reverse` forwards `localhost:34567` directly over USB.
  - **Wireless (Wi-Fi)**: Just connect to the same Wi-Fi network and open your PC's IP address shown in the in-game status popup!

---

## 🚀 Quick Start

### 1. Launch Geometry Dash
- The mod starts the server on port `34567`.
- Click the phone icon badge in the GD Main Menu (bottom bar) or Pause Menu to view your custom URLs.

### 2. Connect Your Phone
- **Wired (USB-C)**:
  1. Turn ON **USB Debugging** in Developer Options.
  2. Plug your phone into your PC with a USB-C cable.
  3. Open your browser and go to: `http://localhost:34567`
- **Wireless (Wi-Fi)**:
  1. Make sure your phone and PC are on the same Wi-Fi network.
  2. In GD, click the status badge to view your PC's local Wi-Fi URL (e.g. `http://192.168.1.50:34567`).
  3. Open that URL in your phone browser!
