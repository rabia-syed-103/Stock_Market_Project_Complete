import json
import websocket
from datetime import datetime

WS_URL = "ws://localhost:8080"

def send_request(ws, request_type, data):
    ws.send(json.dumps({"type": request_type, "data": data}))
    response = ws.recv()
    print(f"→ {request_type}: {data}")
    print(f"← {response}\n")
    return json.loads(response)

# ------------------- Users 11-20 -------------------

def user11():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user11", "balance": 200000})
    buy_orders = [{"symbol": "AAPL", "price": 154.0, "quantity": 10},
                  {"symbol": "GOOG", "price": 2765.0, "quantity": 3}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user12():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user12", "balance": 210000})
    buy_orders = [{"symbol": "MSFT", "price": 307.0, "quantity": 12},
                  {"symbol": "AMZN", "price": 3465.0, "quantity": 5}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user13():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user13", "balance": 220000})
    buy_orders = [{"symbol": "TSLA", "price": 722.0, "quantity": 8},
                  {"symbol": "NFLX", "price": 592.0, "quantity": 6}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user14():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user14", "balance": 240000})
    buy_orders = [{"symbol": "AAPL", "price": 150.0, "quantity": 15},
                  {"symbol": "MSFT", "price": 309.0, "quantity": 7}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user15():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user15", "balance": 230000})
    buy_orders = [{"symbol": "GOOG", "price": 2785.0, "quantity": 4},
                  {"symbol": "AMZN", "price": 3475.0, "quantity": 6}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user16():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user16", "balance": 210000})
    buy_orders = [{"symbol": "TSLA", "price": 718.0, "quantity": 10},
                  {"symbol": "NFLX", "price": 598.0, "quantity": 5}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user17():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user17", "balance": 225000})
    buy_orders = [{"symbol": "AAPL", "price": 151.0, "quantity": 12},
                  {"symbol": "MSFT", "price": 306.0, "quantity": 10}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user18():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user18", "balance": 215000})
    buy_orders = [{"symbol": "GOOG", "price": 2775.0, "quantity": 5},
                  {"symbol": "AMZN", "price": 3480.0, "quantity": 4}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user19():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user19", "balance": 205000})
    buy_orders = [{"symbol": "TSLA", "price": 725.0, "quantity": 7},
                  {"symbol": "NFLX", "price": 600.0, "quantity": 3}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

def user20():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user20", "balance": 230000})
    buy_orders = [{"symbol": "AAPL", "price": 153.0, "quantity": 8},
                  {"symbol": "GOOG", "price": 2790.0, "quantity": 3}]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    ws.close()

# ------------------- Run Users 11-20 -------------------
if __name__ == "__main__":
    print(f"🚀 Starting users 11-20 at {datetime.now()}\n")
    user11()
    user12()
    user13()
    user14()
    user15()
    user16()
    user17()
    user18()
    user19()
    user20()
    print("✅ Users 11-20 completed")
