import websocket
import json
import time

SERVER = "ws://localhost:8080"

def send(ws, payload):
    ws.send(json.dumps(payload))
    response = ws.recv()
    print("→", payload)
    print("←", response)
    print("-" * 50)
    return json.loads(response)

# ------------------ ADMIN SETUP ------------------

ws = websocket.create_connection(SERVER)

# Admin sign in
send(ws, {
    "type": "SIGN_IN",
    "data": {
        "userID": "admin123",
        "balance": 1_000_000
    }
})

# Add stocks
stocks = ["AAPL", "GOOG", "TSLA"]
for s in stocks:
    send(ws, {
        "type": "ADD_STOCK",
        "data": {
            "symbol": s,
            "quantity": 1000
        }
    })

# Admin places SELL orders (initial supply)
admin_orders = [
    ("AAPL", 150, 100),
    ("GOOG", 2800, 50),
    ("TSLA", 700, 80),
]

for symbol, price, qty in admin_orders:
    send(ws, {
        "type": "PLACE_ORDER",
        "data": {
            "symbol": symbol,
            "side": "SELL",
            "price": price,
            "quantity": qty
        }
    })

ws.close()

# ------------------ USERS SETUP ------------------

users = [
    ("alice", 50_000),
    ("bob", 50_000),
    ("charlie", 50_000),
    ("david", 50_000),
    ("eve", 50_000),
]

for user, balance in users:
    ws = websocket.create_connection(SERVER)
    send(ws, {
        "type": "SIGN_IN",
        "data": {
            "userID": user,
            "balance": balance
        }
    })
    ws.close()

print("✅ Market setup complete")
