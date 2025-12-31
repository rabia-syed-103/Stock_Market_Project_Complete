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

# ------------------- User Functions -------------------

def user1():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user1", "balance": 300000})
    
    # BUY orders (some partially match admin123)
    buy_orders = [
        {"symbol": "AAPL", "price": 150.0, "quantity": 30},
        {"symbol": "GOOG", "price": 2750.0, "quantity": 5},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    # SELL orders
    sell_orders = [
        {"symbol": "AAPL", "price": 155.0, "quantity": 10},
    ]
    for order in sell_orders:
        send_request(ws, "PLACE_ORDER", {"side": "SELL", **order})
    
    ws.close()

def user2():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user2", "balance": 250000})
    
    buy_orders = [
        {"symbol": "MSFT", "price": 305.0, "quantity": 25},
        {"symbol": "AMZN", "price": 3460.0, "quantity": 10},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    sell_orders = [
        {"symbol": "MSFT", "price": 310.0, "quantity": 5},
    ]
    for order in sell_orders:
        send_request(ws, "PLACE_ORDER", {"side": "SELL", **order})
    
    ws.close()

def user3():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user3", "balance": 200000})
    
    buy_orders = [
        {"symbol": "TSLA", "price": 720.0, "quantity": 15},
        {"symbol": "NFLX", "price": 590.0, "quantity": 10},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    sell_orders = [
        {"symbol": "TSLA", "price": 730.0, "quantity": 5},
    ]
    for order in sell_orders:
        send_request(ws, "PLACE_ORDER", {"side": "SELL", **order})
    
    ws.close()

def user4():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user4", "balance": 180000})
    
    buy_orders = [
        {"symbol": "AAPL", "price": 152.0, "quantity": 20},
        {"symbol": "GOOG", "price": 2780.0, "quantity": 5},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user5():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user5", "balance": 220000})
    
    buy_orders = [
        {"symbol": "AMZN", "price": 3470.0, "quantity": 8},
        {"symbol": "MSFT", "price": 308.0, "quantity": 12},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user6():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user6", "balance": 260000})
    
    buy_orders = [
        {"symbol": "TSLA", "price": 725.0, "quantity": 10},
        {"symbol": "NFLX", "price": 595.0, "quantity": 6},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user7():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user7", "balance": 240000})
    
    buy_orders = [
        {"symbol": "AAPL", "price": 151.0, "quantity": 15},
        {"symbol": "MSFT", "price": 306.0, "quantity": 10},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user8():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user8", "balance": 210000})
    
    buy_orders = [
        {"symbol": "GOOG", "price": 2770.0, "quantity": 6},
        {"symbol": "AMZN", "price": 3480.0, "quantity": 7},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user9():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user9", "balance": 230000})
    
    buy_orders = [
        {"symbol": "TSLA", "price": 728.0, "quantity": 12},
        {"symbol": "NFLX", "price": 600.0, "quantity": 5},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

def user10():
    ws = websocket.create_connection(WS_URL)
    send_request(ws, "SIGN_IN", {"userID": "user10", "balance": 250000})
    
    buy_orders = [
        {"symbol": "AAPL", "price": 153.0, "quantity": 10},
        {"symbol": "GOOG", "price": 2790.0, "quantity": 4},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    ws.close()

# ------------------- Run All Users -------------------
if __name__ == "__main__":
    print(f"🚀 Starting all 10 users at {datetime.now()}\n")
    user1()
    user2()
    user3()
    user4()
    user5()
    user6()
    user7()
    user8()
    user9()
    user10()
    print("✅ All 10 users completed")
