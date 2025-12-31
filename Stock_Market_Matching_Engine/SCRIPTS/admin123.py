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

def main():
    ws = websocket.create_connection(WS_URL)
    
    print("🚀 Admin123 Setup Started")
    print(f"Started at: {datetime.now()}\n")
    
    # 1️⃣ Sign in admin
    send_request(ws, "SIGN_IN", {"userID": "admin123", "balance": 10000000})
    
    # 2️⃣ Add 10 stocks with quantities
    stocks = {
        "AAPL": 5000,
        "GOOGL": 6000,
        "MSFT": 7000,
        "AMZN": 5500,
        "TSLA": 5800,
        "META": 7200,
        "NVDA": 7300,
        "NFLX": 7400,
        "AMD": 5800,
        "INTC": 9200
    }
    for symbol, qty in stocks.items():
        send_request(ws, "ADD_STOCK", {"symbol": symbol, "quantity": qty})
    
    # 3️⃣ Place SELL orders for all 10 stocks
    sell_orders = [
        {"symbol": "AAPL", "price": 150.0, "quantity": 500},
        {"symbol": "GOOGL", "price": 2800.0, "quantity": 400},
        {"symbol": "MSFT", "price": 380.0, "quantity": 450},
        {"symbol": "AMZN", "price": 175.0, "quantity": 300},
        {"symbol": "TSLA", "price": 250.0, "quantity": 350},
        {"symbol": "META", "price": 485.0, "quantity": 400},
        {"symbol": "NVDA", "price": 870.0, "quantity": 300},
        {"symbol": "NFLX", "price": 650.0, "quantity": 250},
        {"symbol": "AMD", "price": 165.0, "quantity": 200},
        {"symbol": "INTC", "price": 45.0, "quantity": 500},
    ]
    for order in sell_orders:
        send_request(ws, "PLACE_ORDER", {"side": "SELL", **order})
    
    # 4️⃣ Place BUY orders for 5 stocks
    buy_orders = [
        {"symbol": "AAPL", "price": 148.0, "quantity": 100},
        {"symbol": "GOOGL", "price": 2790.0, "quantity": 50},
        {"symbol": "MSFT", "price": 375.0, "quantity": 150},
        {"symbol": "AMZN", "price": 173.0, "quantity": 120},
        {"symbol": "TSLA", "price": 248.0, "quantity": 80},
    ]
    for order in buy_orders:
        send_request(ws, "PLACE_ORDER", {"side": "BUY", **order})
    
    print("✅ Admin123 setup completed successfully.")
    ws.close()

if __name__ == "__main__":
    main()
