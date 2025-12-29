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

def login(user):
    ws = websocket.create_connection(SERVER)
    send(ws, {
        "type": "LOG_IN",
        "data": {
            "userID": user
        }
    })
    return ws

def portfolio(ws):
    send(ws, {
        "type": "GET_PORTFOLIO",
        "data": {}
    })

# ------------------ USERS LOG IN ------------------

alice = login("alice")
bob = login("bob")
charlie = login("charlie")
admin = login("admin123")

# ------------------ BUY FROM ADMIN ------------------

send(alice, {
    "type": "PLACE_ORDER",
    "data": {
        "symbol": "AAPL",
        "side": "BUY",
        "price": 150,
        "quantity": 10
    }
})

portfolio(admin)

send(bob, {
    "type": "PLACE_ORDER",
    "data": {
        "symbol": "TSLA",
        "side": "BUY",
        "price": 700,
        "quantity": 5
    }
})

portfolio(bob)

# ------------------ USER TO USER TRADING ------------------

# Alice sells some AAPL
send(alice, {
    "type": "PLACE_ORDER",
    "data": {
        "symbol": "AAPL",
        "side": "SELL",
        "price": 160,
        "quantity": 4
    }
})

# Charlie buys from Alice
send(charlie, {
    "type": "PLACE_ORDER",
    "data": {
        "symbol": "AAPL",
        "side": "BUY",
        "price": 160,
        "quantity": 4
    }
})

portfolio(alice)
portfolio(charlie)

# ------------------ CLEANUP ------------------

alice.close()
bob.close()
charlie.close()

print("✅ Trading simulation complete")
