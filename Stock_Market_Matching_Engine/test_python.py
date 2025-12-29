import websocket
import json
import time

def on_message(ws, message):
    print("Received:", message)

def on_open(ws):
    print("Connected")

    # Step 1: Sign in alice with balance 1000
    ws.send(json.dumps({
        "type": "SIGN_IN",
        "data": {
            "userID": "bob",
            "balance": 1000
        }
    }))

    # Wait a tiny bit to ensure server processes this first
    time.sleep(0.5)

    ws.send(json.dumps({
       "type": "GET_PORTFOLIO",
        "data": {
            "userID": "bob"
        }
    }))

ws = websocket.WebSocketApp(
    "ws://localhost:8080",
    on_open=on_open,
    on_message=on_message
)

ws.run_forever()
