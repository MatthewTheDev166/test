# Google AI Studio (Build Mode) Prompt: GD Mobile Stats Android Companion App

Copy and paste the entire prompt below into **Google AI Studio** in **Build Mode** to generate the complete Android companion app for **GD Mobile Stats**.

---

```markdown
# Prompt: Build "GD Mobile Stats" Android Companion App

## Overview & Goal
Build a high-performance native Android companion app named **"GD Mobile Stats"** using **Kotlin**, **Jetpack Compose**, and **Material 3**. 
The app acts as an external secondary dashboard ("HUD") and thumb controller for Geometry Dash running on PC.
It connects via a USB-C cable (using ADB reverse port forwarding to `ws://127.0.0.1:34567`) or over local Wi-Fi LAN (`ws://<PC_IP>:34567`), receiving real-time live statistics and sending remote gameplay commands (StartPos switcher and Instant Respawn).

---

## 1. Connection & Networking Specifications

### Protocol
- **WebSocket** via **OkHttp** (`okhttp3:okhttp:4.12.0`).
- Default Target URL for USB-C Cable Mode: `ws://127.0.0.1:34567` (the PC Geode mod runs `adb reverse tcp:34567 tcp:34567`, which routes localhost:34567 over the USB cable directly to the PC).
- Wi-Fi Mode: Supports manual IP / Port input or LAN subnet discovery (e.g., `ws://192.168.1.100:34567`).

### Connection Management
- Three states:
  - `Disconnected` (Red status indicator, showing "Connect" button)
  - `Connecting` (Amber/Yellow pulsing indicator, showing "Connecting...")
  - `Connected` (Vibrant Neon Green indicator with ping/latency display)
- **Auto-Reconnect**: If the socket drops, retry every 3 seconds with exponential backoff up to 10 seconds.
- **Screen Wake Lock**: Acquire `FLAG_KEEP_SCREEN_ON` on the Activity while connected so the device screen never dims or turns off during gaming sessions.

---

## 2. Data & Communication Protocol

### Inbound Packets (PC -> Phone)
The PC Geode mod streams UTF-8 JSON packets at ~20 Hz when in a level or on state changes:

```json
{
  "type": "stats_update",
  "state": "playing", 
  "level": {
    "name": "Bloodlust",
    "creator": "Knobbelboy",
    "id": 42566672,
    "is_practice": false
  },
  "progress": {
    "current_percent": 45.2,
    "best_percent": 78
  },
  "attempts": {
    "total": 12500,
    "session": 34
  },
  "time": {
    "session_seconds": 1285.5
  }
}
```

#### States:
- `"in_menu"`: Player is on the GD title screen or menu.
- `"playing"`: Player is actively in a level.
- `"paused"`: Level is currently paused.
- `"dead"`: Player just crashed.
- `"level_completed"`: Player completed the level (100%).

### Outbound Commands (Phone -> PC)
When the user taps remote control buttons, send lightweight JSON frames:
- **Previous StartPos**: `{"action": "prev_startpos"}` (PC triggers 'Q' key / previous checkpoint)
- **Next StartPos**: `{"action": "next_startpos"}` (PC triggers 'E' key / next checkpoint)
- **Instant Respawn**: `{"action": "respawn"}` (PC triggers 'R' key / resetLevel)
- **Request State Sync**: `{"action": "request_sync"}`

---

## 3. UI/UX & Visual Design (Geometry Dash Cyberpunk/Neon Theme)

### Color Palette
- Background: Deep Dark `#0D1117` / `#161B22`
- Cards / Containers: Dark Slate `#21262D` with subtle `#30363D` borders
- Accents:
  - Neon Cyan `#00FFF0` (Primary accents, progress bar)
  - GD Lime Green `#00FF66` (Connected state, normal mode)
  - Fire Orange `#FF9900` (Attempts, practice mode)
  - Crimson Red `#FF3366` (Death state, disconnect)
  - Electric Purple `#A855F7` (Header, badges)

### Screen 1: Connection & Setup Modal / Top Bar
- Status Pill: `[● CONNECTED - USB]` / `[● WAITING FOR GD]`
- Toggle between:
  - **"USB-C Mode (Recommended)"**: Single tap "Connect to PC" (connects to `127.0.0.1:34567`). Shows a helper tip: *"Plug USB-C cable, make sure USB Debugging is ON in Developer Options"*.
  - **"Wi-Fi LAN Mode"**: Input fields for PC IP Address and Port with "Scan / Connect" button.

### Screen 2: Standby Screen (`state == "in_menu"`)
- Shown when connected but the player is in GD menus.
- Minimalist dashboard:
  - Large stylized text: **"Geometry Dash Menu"**
  - Animated pulsing radar or GD cube logo indicating active connection.
  - Subtitle: *"Waiting for level to start..."*
  - Live clock and connection latency pill.

### Screen 3: Live Gameplay HUD (`state == "playing" | "paused" | "dead" | "level_completed"`)
1. **Header**:
   - Level Name in bold, punchy typography (e.g. **"Bloodlust"**).
   - Creator byline: *"by Knobbelboy"*
   - Badges: `[NORMAL MODE]` (Green) or `[PRACTICE MODE]` (Orange), plus status pill: `PLAYING` / `PAUSED` / `DEAD`.
2. **Main Progress Gauge**:
   - Huge, legible percentage display: **45.2%**
   - Sleek animated glowing progress bar showing current progress vs **Best: 78%**.
3. **Stats Grid (2x2 Cards)**:
   - **Attempts**: `Session: 34` | `Total: 12,500`
   - **Time**: `Session: 21m 25s`
   - **Level ID**: `#42566672`
   - **Status**: Live run status with color animation when dying (flash red) or winning (flash gold).
4. **Remote Control Bar (Bottom)**:
   - Fixed at the bottom for comfortable thumb reach.
   - Three large, tactile, high-contrast buttons:
     - `◀ Prev StartPos` (sends `prev_startpos`)
     - `↺ Respawn` (Accent color, sends `respawn`)
     - `Next StartPos ▶` (sends `next_startpos`)
   - **Haptics**: Trigger quick vibration (`VibrationEffect.createPredefined(EFFECT_CLICK)`) on every tap.

---

## 4. Technical Architecture & Android Implementation

### Architecture
- **Language**: Kotlin 2.x
- **Framework**: Jetpack Compose with Material Design 3
- **Pattern**: Clean MVVM (`GDStatsViewModel`, `WebSocketManager`, `UI State`)
- **JSON Serialization**: Kotlinx Serialization or Gson
- **Permissions**:
  ```xml
  <uses-permission android:name="android.permission.INTERNET" />
  <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />
  <uses-permission android:name="android.permission.VIBRATE" />
  ```

### Dependencies (`app/build.gradle.kts`):
```kotlin
dependencies {
    implementation(platform("androidx.compose:compose-bom:2024.04.01"))
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.compose.ui:ui-tooling-preview")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.8.0")
    implementation("androidx.lifecycle:lifecycle-runtime-compose:2.8.0")
    implementation("com.squareup.okhttp3:okhttp:4.12.0")
    implementation("com.google.code.gson:gson:2.10.1")
}
```

---

## 5. Deliverables Expected
Generate the full, runnable project files:
1. `AndroidManifest.xml` (with permissions and `keepScreenOn`)
2. `WebSocketManager.kt` (Robust OkHttp WebSocket client with auto-reconnect, packet parser, and command dispatch)
3. `GDStatsModel.kt` (Data classes matching the JSON schema)
4. `GDStatsViewModel.kt` (StateFlow holder exposing connection state and stats)
5. `MainActivity.kt` & Compose Screens:
   - `GDStatsApp.kt` (Main container, theme, status bar)
   - `HUDDashboard.kt` (Active level HUD with progress gauge and stats grid)
   - `MenuStandbyScreen.kt` (Standby menu mode)
   - `ControllerBottomBar.kt` (Thumb buttons with haptic feedback)
```
