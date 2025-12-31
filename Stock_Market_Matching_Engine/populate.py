
#!/usr/bin/env python3
"""
Large-Scale Data Population Script for Stock Market Matching Engine
Demonstrates system's ability to handle substantial persistent data
FIXED: Properly maintains WebSocket sessions per your architecture
"""

import websocket
import json
import time
import random
from datetime import datetime

SERVER = "ws://localhost:8080"

# ============================================================================
# CONFIGURATION - SCALE THIS UP TO SHOW PROFESSOR
# ============================================================================

# Stock symbols with realistic price ranges
STOCKS = {
    "AAPL": {"base_price": 150, "volatility": 10},
    "GOOGL": {"base_price": 2800, "volatility": 50},
    "MSFT": {"base_price": 380, "volatility": 15},
    "AMZN": {"base_price": 175, "volatility": 20},
    "TSLA": {"base_price": 250, "volatility": 30},
    "META": {"base_price": 485, "volatility": 25},
    "NVDA": {"base_price": 875, "volatility": 40},
    "NFLX": {"base_price": 650, "volatility": 35},
    "AMD": {"base_price": 165, "volatility": 15},
    "INTC": {"base_price": 45, "volatility": 5},
    "ORCL": {"base_price": 125, "volatility": 10},
    "CRM": {"base_price": 280, "volatility": 20},
    "ADBE": {"base_price": 565, "volatility": 30},
    "PYPL": {"base_price": 65, "volatility": 8},
    "SHOP": {"base_price": 85, "volatility": 12},
    "SQ": {"base_price": 72, "volatility": 10},
    "UBER": {"base_price": 68, "volatility": 8},
    "ABNB": {"base_price": 135, "volatility": 15},
    "COIN": {"base_price": 245, "volatility": 35},
    "RBLX": {"base_price": 42, "volatility": 6},
}

# User profiles - SCALE UP HERE FOR MORE DATA
USER_CONFIGS = {
    "institutional": {
        "count": 10,           # 10 big institutions
        "cash_range": (500000, 2000000),
        "orders_per_user": 100
    },
    "hedge_fund": {
        "count": 15,           # 15 hedge funds
        "cash_range": (200000, 800000),
        "orders_per_user": 80
    },
    "active_trader": {
        "count": 50,           # 50 active traders
        "cash_range": (50000, 200000),
        "orders_per_user": 50
    },
    "retail": {
        "count": 100,          # 100 retail investors
        "cash_range": (5000, 50000),
        "orders_per_user": 30
    },
    "casual": {
        "count": 50,           # 50 casual traders
        "cash_range": (1000, 10000),
        "orders_per_user": 15
    }
}

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def send(ws, payload, show_response=False):
    """Send message and get response"""
    try:
        ws.send(json.dumps(payload))
        response = ws.recv()
        resp_json = json.loads(response)
        
        if show_response:
            print(f"→ {payload['type']}: {payload.get('data', {})}")
            print(f"← {resp_json.get('status', 'unknown')}")
        
        return resp_json
    except Exception as e:
        print(f"❌ Error: {e}")
        return {"status": "error", "message": str(e)}

def generate_price(base_price, volatility):
    """Generate realistic price with variation"""
    variation = random.uniform(-volatility, volatility)
    price = base_price + variation
    return round(max(price, 1), 2)  # Ensure positive price

# ============================================================================
# PHASE 1: ADMIN SETUP
# ============================================================================

def setup_admin_and_stocks():
    """Create admin account and add all stocks to market"""
    print("\n" + "="*80)
    print("PHASE 1: ADMIN SETUP & STOCK LISTING")
    print("="*80)
    
    # Create WebSocket connection for admin
    ws = websocket.create_connection(SERVER)
    
    # Step 1: SIGN_IN creates admin account
    print(f"\n📊 Creating admin account...")
    resp = send(ws, {
        "type": "SIGN_IN",
        "data": {
            "userID": "admin123",
            "balance": 10_000_000  # $10M for admin
        }
    })
    
    if resp.get("status") != "success":
        print(f"❌ Failed to create admin: {resp.get('message')}")
        ws.close()
        return False
    
    print("✅ Admin created with $10,000,000")
    
    # Step 2: ADD_STOCK for each symbol (session is already bound to admin123)
    print(f"\n📈 Adding {len(STOCKS)} stocks to market...")
    for symbol, config in STOCKS.items():
        initial_quantity = random.randint(50000, 100000)
        resp = send(ws, {
            "type": "ADD_STOCK",
            "data": {
                "symbol": symbol,
                "quantity": initial_quantity
            }
        })
        
        if resp.get("status") == "success":
            print(f"  ✓ {symbol}: {initial_quantity:,} shares")
        else:
            print(f"  ✗ {symbol}: {resp.get('message', 'Failed')}")
        
        time.sleep(0.05)  # Small delay
    
    # Close connection
    ws.close()
    print(f"\n✅ Market initialized with {len(STOCKS)} stocks")
    return True

# ============================================================================
# PHASE 2: CREATE USERS
# ============================================================================

def create_all_users():
    """Create all user accounts"""
    print("\n" + "="*80)
    print("PHASE 2: USER ACCOUNT CREATION")
    print("="*80)
    
    all_users = []
    total_count = sum(cfg["count"] for cfg in USER_CONFIGS.values())
    created = 0
    
    for profile_type, config in USER_CONFIGS.items():
        print(f"\n🧑 Creating {config['count']} {profile_type.upper()} accounts...")
        
        for i in range(config["count"]):
            user_id = f"{profile_type}_{i+1}"
            cash = random.randint(*config["cash_range"])
            
            # Each user gets their own connection for SIGN_IN
            ws = websocket.create_connection(SERVER)
            resp = send(ws, {
                "type": "SIGN_IN",
                "data": {
                    "userID": user_id,
                    "balance": cash
                }
            })
            ws.close()
            
            if resp.get("status") == "success":
                all_users.append({
                    "id": user_id,
                    "profile": profile_type,
                    "cash": cash,
                    "orders_target": config["orders_per_user"]
                })
                created += 1
                
                if created % 20 == 0:
                    print(f"  Progress: {created}/{total_count}")
            
            time.sleep(0.02)
    
    print(f"\n✅ Created {len(all_users)} user accounts")
    return all_users

# ============================================================================
# PHASE 3: INITIAL LIQUIDITY (Admin sells stocks)
# ============================================================================

def create_initial_liquidity():
    """Admin places initial sell orders to provide market liquidity"""
    print("\n" + "="*80)
    print("PHASE 3: CREATING INITIAL MARKET LIQUIDITY")
    print("="*80)
    
    # Login as admin (maintains session throughout)
    ws = websocket.create_connection(SERVER)
    send(ws, {
        "type": "LOG_IN",
        "data": {"userID": "admin123"}
    })
    
    orders_placed = 0
    for symbol, config in STOCKS.items():
        base_price = config["base_price"]
        volatility = config["volatility"]
        
        # Create 5-10 sell orders per stock at different prices
        num_orders = random.randint(5, 10)
        
        for _ in range(num_orders):
            price = generate_price(base_price, volatility * 0.3)
            quantity = random.randint(50, 200)
            
            send(ws, {
                "type": "PLACE_ORDER",
                "data": {
                    "symbol": symbol,
                    "side": "SELL",
                    "price": price,
                    "quantity": quantity
                }
            })
            orders_placed += 1
            time.sleep(0.03)
        
        if orders_placed % 20 == 0:
            print(f"  Liquidity orders placed: {orders_placed}")
    
    ws.close()
    print(f"\n✅ Created {orders_placed} liquidity orders across all stocks")

# ============================================================================
# PHASE 4: DISTRIBUTE INITIAL HOLDINGS
# ============================================================================

def distribute_initial_holdings(users):
    """Give users some initial stock positions by buying from admin"""
    print("\n" + "="*80)
    print("PHASE 4: DISTRIBUTING INITIAL HOLDINGS")
    print("="*80)
    
    symbols = list(STOCKS.keys())
    trades_executed = 0
    
    for idx, user in enumerate(users):
        # Login as this user
        ws = websocket.create_connection(SERVER)
        send(ws, {
            "type": "LOG_IN",
            "data": {"userID": user["id"]}
        })
        
        # Each user buys 1-3 different stocks
        num_stocks = random.randint(1, 3)
        stocks_to_buy = random.sample(symbols, num_stocks)
        
        for symbol in stocks_to_buy:
            price = STOCKS[symbol]["base_price"] + random.uniform(-5, 5)
            
            # Quantity based on user profile
            if user["profile"] in ["institutional", "hedge_fund"]:
                quantity = random.randint(50, 200)
            elif user["profile"] == "active_trader":
                quantity = random.randint(20, 100)
            else:
                quantity = random.randint(5, 30)
            
            # Check if user can afford
            cost = price * quantity
            if cost < user["cash"] * 0.3:  # Only use 30% of cash max
                send(ws, {
                    "type": "PLACE_ORDER",
                    "data": {
                        "symbol": symbol,
                        "side": "BUY",
                        "price": price,
                        "quantity": quantity
                    }
                })
                trades_executed += 1
                time.sleep(0.03)
        
        ws.close()
        
        if (idx + 1) % 25 == 0:
            print(f"  Holdings distributed to {idx + 1}/{len(users)} users ({trades_executed} trades)")
    
    print(f"\n✅ Distributed initial holdings via {trades_executed} trades")

# ============================================================================
# PHASE 5: GENERATE MASSIVE TRADING ACTIVITY
# ============================================================================

def generate_trading_activity(users):
    """Generate thousands of orders to simulate realistic trading"""
    print("\n" + "="*80)
    print("PHASE 5: GENERATING MASSIVE TRADING ACTIVITY")
    print("="*80)
    
    symbols = list(STOCKS.keys())
    total_orders = 0
    
    # Shuffle users for random order
    random.shuffle(users)
    
    for idx, user in enumerate(users):
        # Login as this user
        ws = websocket.create_connection(SERVER)
        send(ws, {
            "type": "LOG_IN",
            "data": {"userID": user["id"]}
        })
        
        orders_to_place = user["orders_target"]
        
        for _ in range(orders_to_place):
            symbol = random.choice(symbols)
            side = random.choice(["BUY", "SELL"])
            
            # Generate price near market price
            base = STOCKS[symbol]["base_price"]
            volatility = STOCKS[symbol]["volatility"]
            price = generate_price(base, volatility)
            
            # Quantity based on profile
            if user["profile"] in ["institutional", "hedge_fund"]:
                quantity = random.randint(20, 150)
            elif user["profile"] == "active_trader":
                quantity = random.randint(10, 80)
            else:
                quantity = random.randint(1, 30)
            
            send(ws, {
                "type": "PLACE_ORDER",
                "data": {
                    "symbol": symbol,
                    "side": side,
                    "price": price,
                    "quantity": quantity
                }
            })
            
            total_orders += 1
            
            # Progress updates
            if total_orders % 100 == 0:
                print(f"  Orders placed: {total_orders:,}")
            
            time.sleep(0.01)
        
        ws.close()
        
        if (idx + 1) % 25 == 0:
            print(f"  Processed {idx + 1}/{len(users)} users")
    
    print(f"\n✅ Generated {total_orders:,} orders across all users")

# ============================================================================
# PHASE 6: CREATE SOME CANCELLATIONS
# ============================================================================

def create_cancellations(users):
    """Cancel some random orders to show cancellation feature works"""
    print("\n" + "="*80)
    print("PHASE 6: CREATING ORDER CANCELLATIONS")
    print("="*80)
    
    cancellations = 0
    users_to_cancel = random.sample(users, min(50, len(users)))
    
    for user in users_to_cancel:
        # Login as user
        ws = websocket.create_connection(SERVER)
        send(ws, {
            "type": "LOG_IN",
            "data": {"userID": user["id"]}
        })
        
        # Get portfolio to find active orders
        resp = send(ws, {
            "type": "GET_PORTFOLIO",
            "data": {}
        })
        
        if resp.get("status") == "success" and resp.get("data"):
            active_orders = resp["data"].get("activeOrders", [])
            
            if active_orders:
                # Cancel 20-50% of active orders
                num_to_cancel = max(1, len(active_orders) // random.randint(2, 5))
                orders_to_cancel = random.sample(active_orders, min(num_to_cancel, len(active_orders)))
                
                for order in orders_to_cancel:
                    send(ws, {
                        "type": "CANCEL_ORDER",
                        "data": {
                            "orderID": order["orderID"]
                        }
                    })
                    cancellations += 1
                    time.sleep(0.02)
        
        ws.close()
    
    print(f"\n✅ Cancelled {cancellations} orders")

# ============================================================================
# PHASE 7: FINAL STATISTICS
# ============================================================================

def show_final_statistics(users):
    """Display final system statistics"""
    print("\n" + "="*80)
    print("PHASE 7: FINAL SYSTEM STATISTICS")
    print("="*80)
    
    # Sample a few users to show their portfolios
    sample_users = random.sample(users, min(5, len(users)))
    
    print("\n📊 Sample User Portfolios:")
    for user in sample_users:
        ws = websocket.create_connection(SERVER)
        send(ws, {
            "type": "LOG_IN",
            "data": {"userID": user["id"]}
        })
        
        resp = send(ws, {
            "type": "GET_PORTFOLIO",
            "data": {}
        })
        
        if resp.get("status") == "success" and resp.get("data"):
            data = resp["data"]
            cash = data.get("cash", {}).get("available", 0)
            holdings = data.get("holdings", {})
            active_orders = len(data.get("activeOrders", []))
            
            print(f"\n  User: {user['id']}")
            print(f"    Cash: ${cash:,.2f}")
            print(f"    Holdings: {len(holdings)} different stocks")
            print(f"    Active Orders: {active_orders}")
        
        ws.close()
    
    # Show market statistics
    print("\n📈 Market Overview:")
    ws = websocket.create_connection(SERVER)
    send(ws, {
        "type": "LOG_IN",
        "data": {"userID": "admin123"}
    })
    
    for symbol in list(STOCKS.keys())[:5]:  # Show first 5
        resp = send(ws, {
            "type": "GET_ORDERBOOK",
            "data": {"symbol": symbol}
        })
        
        if resp.get("status") == "success" and resp.get("data"):
            book = resp["data"]
            num_bids = len(book.get("buy", []))
            num_asks = len(book.get("sell", []))
            print(f"  {symbol}: {num_bids} bid levels, {num_asks} ask levels")
    
    ws.close()

# ============================================================================
# MAIN EXECUTION
# ============================================================================

def main():
    """Main execution function"""
    print("\n" + "="*80)
    print("🚀 LARGE-SCALE DATA POPULATION STARTED")
    print("="*80)
    print(f"Started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    start_time = time.time()
    
    try:
        # Phase 1: Setup
        if not setup_admin_and_stocks():
            print("❌ Failed to setup admin. Exiting.")
            return
        time.sleep(1)
        
        # Phase 2: Create users
        users = create_all_users()
        if not users:
            print("❌ No users created. Exiting.")
            return
        time.sleep(1)
        
        # Phase 3: Initial liquidity
        create_initial_liquidity()
        time.sleep(1)
        
        # Phase 4: Distribute holdings
        distribute_initial_holdings(users)
        time.sleep(1)
        
        # Phase 5: Generate massive trading
        generate_trading_activity(users)
        time.sleep(1)
        
        # Phase 6: Create cancellations
        create_cancellations(users)
        time.sleep(1)
        
        # Phase 7: Show statistics
        show_final_statistics(users)
        
        # Final summary
        elapsed = time.time() - start_time
        total_users = len(users) + 1  # +1 for admin
        estimated_orders = sum(u["orders_target"] for u in users) + 100  # +100 for admin liquidity
        
        print("\n" + "="*80)
        print("✅ DATA POPULATION COMPLETE")
        print("="*80)
        print(f"Completed at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Time taken: {elapsed:.2f} seconds")
        print(f"\n📊 Summary:")
        print(f"  • Users created: {total_users}")
        print(f"  • Stocks listed: {len(STOCKS)}")
        print(f"  • Estimated orders: {estimated_orders:,}+")
        print(f"  • Data files populated: orders.dat, users.dat, trades.dat, symbols.dat")
        print(f"\n💾 Your system now has substantial persistent data!")
        print(f"🎓 Perfect for demonstrating to your professor!")
        print("="*80 + "\n")
        
    except Exception as e:
        print(f"\n❌ Error during population: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()