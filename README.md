# GD Mobile Stats 📱⚡

A Geode mod for **Geometry Dash** that turns your Android phone into an external live HUD and remote controller over a USB-C cable or local Wi-Fi.

---

## ✨ Features
- **Live Stats Streaming**:
  - Current % with real-time decimal precision.
  - Level Name, Creator, and Level ID.
  - Total Attempts & Current Session Attempts.
  - Session Play Time.
  - Game State (Main Menu, Playing, Paused, Dead, Level Complete).
- **StartPos Remote Switcher & Respawn**:
  - Switch start positions (`Q` / `E`) directly from your phone screen.
  - Quick Respawn / Kill button (`R`) to restart runs instantly.
  - Compatible with **MegaHack** and other keybind-based mods.
- **Instant USB-C Connection**:
  - Automatically manages `adb reverse tcp:34567 tcp:34567` in the background using bundled ADB tools.
  - Simply connect your phone via USB-C with USB debugging enabled!
- **In-Game Status Indicator**:
  - Colored indicator dot in the GD Main Menu and Pause Menu showing connection state.
  - Click to view port, connected devices, and ADB status popup.

---

## 🚀 Quick Setup & Usage

### 1. In Geometry Dash (PC)
1. Install the mod via Geode.
2. In mod settings, verify port (default: `34567`) and ensure **Enable USB ADB Reverse** is ON.
3. When you open Geometry Dash, the WebSocket server starts automatically.

### 2. Connect Your Phone
1. Enable **Developer Options** and turn ON **USB Debugging** on your Android phone.
2. Plug your phone into your PC using a USB-C cable.
3. The mod automatically runs `adb reverse`, routing `localhost:34567` over the USB cable directly to the PC!

---

## 🛠 Testing Before Mobile Build
You can test the connection immediately using the included Python client:
```bash
pip install websocket-client
python test_client.py
```
This connects to `ws://127.0.0.1:34567`, displays live stats from GD in real-time, and lets you press `q`, `e`, or `r` to test the remote controls!

---

## 📱 Generating the Android App
We have provided an engineered prompt tailored for **Google AI Studio (Build Mode)** in:
👉 [`android-prompt/AI_STUDIO_BUILD_PROMPT.md`](android-prompt/AI_STUDIO_BUILD_PROMPT.md)

1. Open [Google AI Studio](https://aistudio.google.com).
2. Switch to **Build Mode**.
3. Copy and paste the prompt from `AI_STUDIO_BUILD_PROMPT.md`.
4. AI Studio will generate the full Android Studio project in Kotlin & Jetpack Compose!
