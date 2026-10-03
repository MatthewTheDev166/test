#pragma once
#include <string_view>

namespace WebHUD {
    constexpr std::string_view HTML_CONTENT = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no, viewport-fit=cover">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="theme-color" content="#090c10">
<title>GD Mobile Stats HUD</title>
<style>
  :root {
    --bg-main: #090c10;
    --bg-card: rgba(22, 27, 34, 0.85);
    --border: rgba(48, 54, 61, 0.7);
    --cyan: #00fff0;
    --cyan-glow: rgba(0, 255, 240, 0.35);
    --green: #00ff66;
    --green-glow: rgba(0, 255, 102, 0.35);
    --orange: #ff9900;
    --red: #ff3366;
    --red-glow: rgba(255, 51, 102, 0.4);
    --gold: #ffd700;
    --text: #f0f6fc;
    --text-muted: #8b949e;
  }

  * {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
    -webkit-tap-highlight-color: transparent;
    user-select: none;
    -webkit-user-select: none;
  }

  body {
    background-color: var(--bg-main);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", sans-serif;
    min-height: 100vh;
    min-height: 100dvh;
    display: flex;
    flex-direction: column;
    overflow-x: hidden;
    touch-action: manipulation;
  }

  /* Cyberpunk Background Grid */
  body::before {
    content: "";
    position: fixed;
    top: 0; left: 0; right: 0; bottom: 0;
    background-image: 
      linear-gradient(rgba(0, 255, 240, 0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(0, 255, 240, 0.03) 1px, transparent 1px);
    background-size: 30px 30px;
    pointer-events: none;
    z-index: 0;
  }

  /* Top Bar */
  .top-bar {
    position: relative;
    z-index: 2;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px 16px;
    background: rgba(13, 17, 23, 0.8);
    backdrop-filter: blur(10px);
    border-bottom: 1px solid var(--border);
  }

  .brand {
    display: flex;
    align-items: center;
    gap: 8px;
    font-weight: 800;
    font-size: 15px;
    letter-spacing: 1px;
    color: var(--cyan);
    text-transform: uppercase;
  }

  .status-badge {
    display: flex;
    align-items: center;
    gap: 6px;
    font-size: 12px;
    font-weight: 600;
    padding: 4px 10px;
    border-radius: 20px;
    background: rgba(0, 0, 0, 0.4);
    border: 1px solid var(--border);
  }

  .status-dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: #666;
    transition: background 0.3s;
  }
  .status-dot.connected {
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
  }

  .btn-fs {
    background: transparent;
    border: 1px solid var(--border);
    color: var(--text-muted);
    font-size: 12px;
    padding: 4px 8px;
    border-radius: 6px;
    cursor: pointer;
  }

  /* Container */
  .main-content {
    position: relative;
    z-index: 1;
    flex: 1;
    display: flex;
    flex-direction: column;
    padding: 14px;
    gap: 14px;
    max-width: 900px;
    width: 100%;
    margin: 0 auto;
  }

  /* Standby Screen */
  #standby-screen {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    text-align: center;
    gap: 14px;
    padding: 20px;
  }

  .radar-ring {
    width: 90px;
    height: 90px;
    border-radius: 50%;
    border: 2px solid var(--cyan);
    position: relative;
    display: flex;
    align-items: center;
    justify-content: center;
    box-shadow: 0 0 20px var(--cyan-glow);
    animation: pulse 2s infinite ease-in-out;
  }

  @keyframes pulse {
    0% { transform: scale(0.95); opacity: 0.7; }
    50% { transform: scale(1.05); opacity: 1; box-shadow: 0 0 35px var(--cyan-glow); }
    100% { transform: scale(0.95); opacity: 0.7; }
  }

  .radar-icon {
    font-size: 36px;
  }

  /* HUD Active Screen */
  #hud-screen {
    display: none;
    flex-direction: column;
    gap: 14px;
    flex: 1;
  }

  /* Level Header Card */
  .level-card {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 16px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.4);
  }

  .level-info {
    overflow: hidden;
  }

  .level-name {
    font-size: 22px;
    font-weight: 800;
    color: #fff;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    letter-spacing: 0.5px;
  }

  .creator-name {
    font-size: 13px;
    color: var(--text-muted);
    margin-top: 2px;
  }

  .mode-pill {
    padding: 4px 10px;
    border-radius: 8px;
    font-size: 11px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.5px;
    background: rgba(0, 255, 102, 0.15);
    color: var(--green);
    border: 1px solid rgba(0, 255, 102, 0.4);
  }
  .mode-pill.practice {
    background: rgba(255, 153, 0, 0.15);
    color: var(--orange);
    border-color: rgba(255, 153, 0, 0.4);
  }

  /* Progress Display */
  .progress-card {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 18px;
    display: flex;
    flex-direction: column;
    gap: 10px;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.4);
  }

  .percent-row {
    display: flex;
    justify-content: space-between;
    align-items: baseline;
  }

  .percent-main {
    font-size: 48px;
    font-weight: 900;
    color: var(--cyan);
    text-shadow: 0 0 25px var(--cyan-glow);
    font-variant-numeric: tabular-nums;
  }

  .best-percent {
    font-size: 14px;
    color: var(--text-muted);
    font-weight: 600;
  }
  .best-percent span {
    color: var(--gold);
    font-weight: 700;
  }

  .progress-track {
    width: 100%;
    height: 14px;
    background: rgba(0, 0, 0, 0.5);
    border-radius: 10px;
    overflow: hidden;
    border: 1px solid rgba(255, 255, 255, 0.1);
    position: relative;
  }

  .progress-fill {
    height: 100%;
    width: 0%;
    background: linear-gradient(90deg, #00d2ff, var(--cyan));
    box-shadow: 0 0 15px var(--cyan);
    border-radius: 10px;
    transition: width 0.1s linear;
  }

  /* Stats Grid */
  .stats-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 10px;
  }

  .stat-box {
    background: var(--bg-card);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 12px;
    text-align: center;
  }

  .stat-label {
    font-size: 11px;
    color: var(--text-muted);
    text-transform: uppercase;
    font-weight: 700;
    margin-bottom: 4px;
  }

  .stat-val {
    font-size: 18px;
    font-weight: 800;
    color: #fff;
    font-variant-numeric: tabular-nums;
  }

  /* Status Banner */
  .state-banner {
    padding: 8px 14px;
    border-radius: 10px;
    font-size: 13px;
    font-weight: 800;
    text-align: center;
    letter-spacing: 1px;
    text-transform: uppercase;
    transition: all 0.25s;
  }
  .state-banner.playing {
    background: rgba(0, 255, 102, 0.12);
    color: var(--green);
    border: 1px solid rgba(0, 255, 102, 0.3);
  }
  .state-banner.dead {
    background: rgba(255, 51, 102, 0.15);
    color: var(--red);
    border: 1px solid rgba(255, 51, 102, 0.4);
    box-shadow: 0 0 15px var(--red-glow);
  }
  .state-banner.paused {
    background: rgba(255, 153, 0, 0.15);
    color: var(--orange);
    border: 1px solid rgba(255, 153, 0, 0.4);
  }
  .state-banner.level_completed {
    background: rgba(255, 215, 0, 0.2);
    color: var(--gold);
    border: 1px solid var(--gold);
    box-shadow: 0 0 20px rgba(255, 215, 0, 0.4);
  }

  /* Remote Controller Buttons */
  .controller-bar {
    display: grid;
    grid-template-columns: 1fr 1.2fr 1fr;
    gap: 12px;
    margin-top: auto;
    padding-top: 10px;
  }

  .ctrl-btn {
    appearance: none;
    border: none;
    outline: none;
    border-radius: 14px;
    padding: 16px 8px;
    font-size: 15px;
    font-weight: 800;
    color: #fff;
    cursor: pointer;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 4px;
    transition: transform 0.08s, filter 0.08s;
    box-shadow: 0 4px 15px rgba(0, 0, 0, 0.5);
  }

  .ctrl-btn:active {
    transform: scale(0.94);
    filter: brightness(1.2);
  }

  .btn-pos {
    background: linear-gradient(135deg, #252d3d, #1c2331);
    border: 1px solid rgba(0, 255, 240, 0.3);
  }
  .btn-pos .btn-sub {
    font-size: 10px;
    color: var(--cyan);
    font-weight: 700;
  }

  .btn-respawn {
    background: linear-gradient(135deg, #d62828, #9e1b1b);
    border: 1px solid rgba(255, 51, 102, 0.5);
    box-shadow: 0 4px 18px var(--red-glow);
  }
  .btn-respawn .btn-sub {
    font-size: 10px;
    color: rgba(255, 255, 255, 0.8);
    font-weight: 700;
  }

  /* ----------------- LANDSCAPE MODE ----------------- */
  @media (orientation: landscape) and (max-height: 550px) {
    .top-bar {
      padding: 6px 14px;
    }
    .main-content {
      padding: 8px 12px;
      gap: 8px;
    }
    #hud-screen {
      display: none;
      flex-direction: row;
      gap: 12px;
    }
    .hud-col-left {
      flex: 1.1;
      display: flex;
      flex-direction: column;
      gap: 8px;
    }
    .hud-col-right {
      flex: 1;
      display: flex;
      flex-direction: column;
      gap: 8px;
    }
    .level-card {
      padding: 10px 14px;
    }
    .level-name {
      font-size: 18px;
    }
    .progress-card {
      padding: 10px 14px;
      gap: 6px;
    }
    .percent-main {
      font-size: 38px;
    }
    .stats-grid {
      grid-template-columns: repeat(3, 1fr);
      gap: 6px;
    }
    .stat-box {
      padding: 6px;
    }
    .stat-val {
      font-size: 15px;
    }
    .controller-bar {
      margin-top: auto;
      padding-top: 0;
      gap: 8px;
    }
    .ctrl-btn {
      padding: 12px 6px;
      font-size: 14px;
    }
  }
</style>
</head>
<body>

  <!-- Top Bar -->
  <div class="top-bar">
    <div class="brand">
      <span>⚡ GD HUD</span>
    </div>
    <div style="display: flex; align-items: center; gap: 10px;">
      <div class="status-badge">
        <div class="status-dot" id="status-dot"></div>
        <span id="status-text">Connecting...</span>
      </div>
      <button class="btn-fs" id="fs-btn" onclick="toggleFullscreen()">⛶ Fullscreen</button>
    </div>
  </div>

  <div class="main-content">
    
    <!-- Standby / In Menu Screen -->
    <div id="standby-screen">
      <div class="radar-ring">
        <span class="radar-icon">🎮</span>
      </div>
      <h2 style="font-weight: 800; font-size: 20px; color: #fff;">GD Main Menu</h2>
      <p style="color: var(--text-muted); font-size: 13px; max-width: 260px;">
        Geometry Dash is in menu. Select or start a level to stream live stats!
      </p>
    </div>

    <!-- Active Gameplay HUD Screen -->
    <div id="hud-screen">
      <div class="hud-col-left">
        <!-- Level Info -->
        <div class="level-card">
          <div class="level-info">
            <div class="level-name" id="level-name">Stereo Madness</div>
            <div class="creator-name" id="creator-name">by RobTop</div>
          </div>
          <div class="mode-pill" id="mode-pill">NORMAL</div>
        </div>

        <!-- Progress Gauge -->
        <div class="progress-card">
          <div class="percent-row">
            <div class="percent-main" id="percent-val">0.0%</div>
            <div class="best-percent">Best: <span id="best-val">0%</span></div>
          </div>
          <div class="progress-track">
            <div class="progress-fill" id="progress-fill"></div>
          </div>
        </div>

        <!-- Live Status Banner -->
        <div class="state-banner playing" id="state-banner">PLAYING</div>
      </div>

      <div class="hud-col-right">
        <!-- Stats Grid -->
        <div class="stats-grid">
          <div class="stat-box">
            <div class="stat-label">Session</div>
            <div class="stat-val" id="sess-att">0</div>
          </div>
          <div class="stat-box">
            <div class="stat-label">Total Att</div>
            <div class="stat-val" id="tot-att">0</div>
          </div>
          <div class="stat-box">
            <div class="stat-label">Time</div>
            <div class="stat-val" id="time-val">00:00</div>
          </div>
        </div>

        <!-- Controller Buttons -->
        <div class="controller-bar">
          <button class="ctrl-btn btn-pos" onclick="sendAction('prev_startpos')">
            <span>◀ Prev</span>
            <span class="btn-sub">KEY Q</span>
          </button>
          <button class="ctrl-btn btn-respawn" onclick="sendAction('respawn')">
            <span>↺ Respawn</span>
            <span class="btn-sub">KEY R</span>
          </button>
          <button class="ctrl-btn btn-pos" onclick="sendAction('next_startpos')">
            <span>Next ▶</span>
            <span class="btn-sub">KEY E</span>
          </button>
        </div>
      </div>
    </div>

  </div>

  <script>
    let ws = null;
    let wakeLock = null;

    // Wake Lock to keep phone screen awake
    async function requestWakeLock() {
      try {
        if ('wakeLock' in navigator) {
          wakeLock = await navigator.wakeLock.request('screen');
        }
      } catch (err) {}
    }
    document.addEventListener('click', requestWakeLock);
    document.addEventListener('touchstart', requestWakeLock);
    document.addEventListener('visibilitychange', () => {
      if (document.visibilityState === 'visible') requestWakeLock();
    });

    // Fullscreen toggle
    function toggleFullscreen() {
      if (!document.fullscreenElement) {
        document.documentElement.requestFullscreen().catch(() => {});
        document.getElementById('fs-btn').innerText = '✕ Exit';
      } else {
        if (document.exitFullscreen) document.exitFullscreen();
        document.getElementById('fs-btn').innerText = '⛶ Fullscreen';
      }
    }

    // Vibration Haptics & WebSocket Commands
    function sendAction(action) {
      if (navigator.vibrate) {
        navigator.vibrate(35);
      }
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ action: action }));
      }
    }

    // Keyboard support for desktop testing
    window.addEventListener('keydown', (e) => {
      if (e.key === 'q' || e.key === 'Q') sendAction('prev_startpos');
      if (e.key === 'e' || e.key === 'E') sendAction('next_startpos');
      if (e.key === 'r' || e.key === 'R') sendAction('respawn');
    });

    function formatTime(seconds) {
      const s = Math.floor(seconds);
      const mins = Math.floor(s / 60);
      const remSecs = s % 60;
      return String(mins).padStart(2, '0') + ':' + String(remSecs).padStart(2, '0');
    }

    function connect() {
      const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
      const wsUrl = proto + '//' + location.host;

      const dot = document.getElementById('status-dot');
      const text = document.getElementById('status-text');

      ws = new WebSocket(wsUrl);

      ws.onopen = () => {
        dot.className = 'status-dot connected';
        text.innerText = 'Connected';
        ws.send(JSON.stringify({ action: 'request_sync' }));
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          updateUI(data);
        } catch (e) {}
      };

      ws.onclose = () => {
        dot.className = 'status-dot';
        text.innerText = 'Disconnected';
        setTimeout(connect, 1500);
      };

      ws.onerror = () => {
        ws.close();
      };
    }

    function updateUI(data) {
      const state = data.state || 'in_menu';
      const standby = document.getElementById('standby-screen');
      const hud = document.getElementById('hud-screen');

      if (state === 'in_menu') {
        standby.style.display = 'flex';
        hud.style.display = 'none';
        return;
      }

      standby.style.display = 'none';
      hud.style.display = 'flex';

      // Level
      const lvl = data.level || {};
      document.getElementById('level-name').innerText = lvl.name || 'Unknown Level';
      document.getElementById('creator-name').innerText = 'by ' + (lvl.creator || 'RobTop');

      const modePill = document.getElementById('mode-pill');
      if (lvl.is_practice) {
        modePill.innerText = 'PRACTICE';
        modePill.className = 'mode-pill practice';
      } else {
        modePill.innerText = 'NORMAL';
        modePill.className = 'mode-pill';
      }

      // Progress
      const prog = data.progress || {};
      const pct = (prog.current_percent || 0.0);
      document.getElementById('percent-val').innerText = pct.toFixed(1) + '%';
      document.getElementById('best-val').innerText = (prog.best_percent || 0) + '%';
      document.getElementById('progress-fill').style.width = Math.min(100, Math.max(0, pct)) + '%';

      // Stats
      const att = data.attempts || {};
      document.getElementById('sess-att').innerText = att.session || 0;
      document.getElementById('tot-att').innerText = att.total || 0;

      const t = data.time || {};
      document.getElementById('time-val').innerText = formatTime(t.session_seconds || 0);

      // State Banner
      const banner = document.getElementById('state-banner');
      banner.className = 'state-banner ' + state;
      if (state === 'dead') {
        banner.innerText = 'CRASHED / DEAD';
      } else if (state === 'paused') {
        banner.innerText = 'PAUSED';
      } else if (state === 'level_completed') {
        banner.innerText = '100% COMPLETE!';
      } else {
        banner.innerText = 'PLAYING';
      }
    }

    connect();
  </script>
</body>
</html>)rawhtml";
}
