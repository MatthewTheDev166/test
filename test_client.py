"""
Standalone Test Client for GD Mobile Stats
Use this to test the WebSocket connection and StartPos switcher commands
before running on your Android phone!
"""

import sys
import json
import time

try:
    import websocket
except ImportError:
    print("Please install websocket-client first: pip install websocket-client")
    sys.exit(1)

SERVER_URL = "ws://127.0.0.1:34567"

def on_message(ws, message):
    try:
        data = json.loads(message)
        state = data.get("state", "unknown")
        
        if state == "in_menu":
            print(f"[GD] State: IN MAIN MENU")
        else:
            lvl = data.get("level", {})
            prog = data.get("progress", {})
            att = data.get("attempts", {})
            t = data.get("time", {})
            name = lvl.get("name", "Unknown")
            creator = lvl.get("creator", "Unknown")
            pct = prog.get("current_percent", 0.0)
            best = prog.get("best_percent", 0)
            tot_att = att.get("total", 0)
            sess_att = att.get("session", 0)
            sess_time = t.get("session_seconds", 0.0)
            
            print(f"[{state.upper()}] {name} by {creator} | Current: {pct:.1f}% (Best: {best}%) | Att: {sess_att}/{tot_att} | Time: {int(sess_time)}s")
    except Exception as e:
        print(f"Received raw: {message}")

def on_error(ws, error):
    print(f"[Error] {error}")

def on_close(ws, close_status_code, close_msg):
    print("[Connection Closed]")

def on_open(ws):
    print(f"[Connected to GD Mobile Stats at {SERVER_URL}]")
    print("Commands:")
    print("  Type 'q' and Enter -> Prev StartPos")
    print("  Type 'e' and Enter -> Next StartPos")
    print("  Type 'r' and Enter -> Instant Respawn")
    print("  Type 'sync' and Enter -> Request full state sync")
    print("-" * 50)
    
    # Send sync request
    ws.send(json.dumps({"action": "request_sync"}))

    def input_loop():
        while True:
            cmd = input().strip().lower()
            if cmd == 'q':
                ws.send(json.dumps({"action": "prev_startpos"}))
                print(">> Sent Prev StartPos (Q)")
            elif cmd == 'e':
                ws.send(json.dumps({"action": "next_startpos"}))
                print(">> Sent Next StartPos (E)")
            elif cmd == 'r':
                ws.send(json.dumps({"action": "respawn"}))
                print(">> Sent Instant Respawn (R)")
            elif cmd == 'sync':
                ws.send(json.dumps({"action": "request_sync"}))
                print(">> Sent Request Sync")
            elif cmd == 'exit':
                ws.close()
                break

    import threading
    t = threading.Thread(target=input_loop, daemon=True)
    t.start()

if __name__ == "__main__":
    print(f"Connecting to {SERVER_URL}...")
    ws = websocket.WebSocketApp(
        SERVER_URL,
        on_open=on_open,
        on_message=on_message,
        on_error=on_error,
        on_close=on_close
    )
    ws.run_forever()
