#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include "OrderBook.h"
#include "../core/User.h"
#include "../data_structures/MyHashMap.h"

using namespace std;

class MatchingEngine {
private:

    MyHashMap<string, OrderBook*>* orderBooks;
    MyHashMap<int, Order*>* allOrders;
    MyHashMap<string, User*>* users;
    
    vector<Trade> tradeHistory;
    
    mutex engineLock;
    mutex tradeLock;
    mutex userLock;
    
    int nextOrderID;
    int nextTradeID;

public:
    // ✅ FIXED CONSTRUCTOR
    MatchingEngine() : nextOrderID(1), nextTradeID(1) {
        // ✅ Allocate on heap and assign to member variables
        orderBooks = new MyHashMap<string, OrderBook*>(100);
        allOrders = new MyHashMap<int, Order*>(10000);
        users = new MyHashMap<string, User*>(1000);
        
        // ✅ Now this works!
        orderBooks->insert("AAPL", new OrderBook("AAPL"));
        orderBooks->insert("GOOGL", new OrderBook("GOOGL"));
        orderBooks->insert("TSLA", new OrderBook("TSLA"));
        orderBooks->insert("MSFT", new OrderBook("MSFT"));
    }
    
    ~MatchingEngine() {
        // ✅ Clean up
        delete orderBooks;
        delete allOrders;
        delete users;
    }
    
   
    void createUser(string userID, double initialCash) {
        lock_guard<mutex> lock(userLock);
        
        User* existing = users->get(userID);
        if (existing) {
            cout << "User " << userID << " already exists\n";
            return;
        }
        
        User* user = new User(userID, initialCash);
        users->insert(userID, user);
        
        cout << "Created user " << userID << " with $" << initialCash << "\n";
    }
    
    User* getUser(string userID) {
        lock_guard<mutex> lock(userLock);
        return users->get(userID);
    }
  
    Order* placeOrder(string userID, string symbol, 
                      string side, double price, int quantity) {
        
        // Step 1: Validate user balance
        {
            lock_guard<mutex> lock(userLock);
            
            User* user = users->get(userID);
            if (!user) {
                cout << "Error: User " << userID << " not found\n";
                return nullptr;
            }
            
            if (side == "BUY") {
                double cost = price * quantity;
                if (user->cashBalance < cost) {
                    cout << "Error: Insufficient funds. Need $" << cost 
                         << ", have $" << user->cashBalance << "\n";
                    return nullptr;
                }
                user->deductCash(cost);
            } else {
                if (!user->removeStock(symbol, quantity)) {
                    cout << "Error: Insufficient shares of " << symbol << "\n";
                    return nullptr;
                }
            }
        }
        
        // Step 2: Create order
        Order* order;
        {
            lock_guard<mutex> lock(engineLock);
            
            int orderID = nextOrderID++;
            order = new Order(orderID, userID, symbol, side, price, quantity);
            allOrders->insert(orderID, order);
        }
        
        // Step 3: Get order book
        OrderBook* book;
        {
            lock_guard<mutex> lock(engineLock);
            book = orderBooks->get(symbol);
            
            if (!book) {
                book = new OrderBook(symbol);
                orderBooks->insert(symbol, book);
            }
        }
        
        // Step 4: Match order
        vector<Trade> trades = book->addOrder(order);
        
        // Step 5: Process trades
        for (Trade& trade : trades) {
            // Assign trade ID
            {
                lock_guard<mutex> lock(engineLock);
                trade.tradeID = nextTradeID++;
            }
            
            // Update user portfolios
            {
                lock_guard<mutex> lock(userLock);
                
                User* buyer = users->get(trade.buyUserID);
                User* seller = users->get(trade.sellUserID);
                
                if (buyer && seller) {
                    buyer->addStock(symbol, trade.quantity);
                    seller->addCash(trade.price * trade.quantity);
                    
                    cout << "Trade executed: " << trade.toString() << "\n";
                }
            }
            
            // Record trade
            {
                lock_guard<mutex> lock(tradeLock);
                tradeHistory.push_back(trade);
            }
        }
        
        // Step 6: Refund if not fully filled
        if (order->status != "FILLED") {
            lock_guard<mutex> lock(userLock);
            
            User* user = users->get(userID);
            if (side == "BUY") {
                user->addCash(price * order->remainingQty);
            } else {
                user->addStock(symbol, order->remainingQty);
            }
        }
        
        return order;
    }
    
 
    void cancelOrder(int orderID, string userID) {
        Order* order;
        string symbol;
        string side;
        int remaining;
        double price;
        
        // Get order info
        {
            lock_guard<mutex> lock(engineLock);
            
            order = allOrders->get(orderID);
            if (!order) {
                cout << "Error: Order " << orderID << " not found\n";
                return;
            }
            
            if (order->userID != userID) {
                cout << "Error: Order " << orderID << " does not belong to " << userID << "\n";
                return;
            }
            
            symbol = order->symbol;
            side = order->side;
            remaining = order->remainingQty;
            price = order->price;
        }
        
        // Cancel in order book
        OrderBook* book;
        {
            lock_guard<mutex> lock(engineLock);
            book = orderBooks->get(symbol);
        }
        
        if (book) {
            book->cancelOrder(orderID);
        }
        
        // Refund user
        {
            lock_guard<mutex> lock(userLock);
            
            User* user = users->get(userID);
            if (user && remaining > 0) {
                if (side == "BUY") {
                    user->addCash(price * remaining);
                } else {
                    user->addStock(symbol, remaining);
                }
                user->removeActiveOrder(orderID);
            }
        }
    }
    
    
    vector<Trade> getAllTrades() {
        lock_guard<mutex> lock(tradeLock);
        return tradeHistory;
    }
    
    Order* getOrder(int orderID) {
        lock_guard<mutex> lock(engineLock);
        return allOrders->get(orderID);
    }
    
    OrderBook* getOrderBook(string symbol) {
        lock_guard<mutex> lock(engineLock);
        return orderBooks->get(symbol);
    }
    
    void printOrderBook(const std::string& symbol) {
    // Lock engine to safely access orderBooks map
    lock_guard<mutex> lock(engineLock);
    
    // Get the order book for the symbol
    OrderBook* book = orderBooks->get(symbol);
    if (!book) {
        cout << "No order book for symbol: " << symbol << "\n";
        return;
    }
    
    // Lock the order book itself while printing
    book->printOrderBook();
}

    string getPortfolio(string userID) {
        lock_guard<mutex> lock(userLock);
        
        User* user = users->get(userID);
        if (!user) {
            return "User not found";
        }
        
        return user->toString();
    }
    
    // Get user's cash balance
    double getCashBalance(string userID) {
        lock_guard<mutex> lock(userLock);
        
        User* user = users->get(userID);
        if (!user) return -1;
        
        return user->cashBalance;
    }
    
    // Get user's stock holdings
    struct Holding {
        string symbol;
        int quantity;
    };
    
    vector<Holding> getHoldings(string userID) {
        lock_guard<mutex> lock(userLock);
        
        vector<Holding> holdings;
        User* user = users->get(userID);
        
        if (!user) return holdings;
        
        for (int i = 0; i < user->holdingsCount; i++) {
            Holding h;
            h.symbol = user->symbols[i];
            h.quantity = user->quantities[i];
            holdings.push_back(h);
        }
        
        return holdings;
    }
    
    // Get user's active orders
    vector<Order*> getActiveOrders(string userID) {
        lock_guard<mutex> lock(userLock);
        
        vector<Order*> orders;
        User* user = users->get(userID);
        
        if (!user) return orders;
        
        lock_guard<mutex> engineLk(engineLock);
        for (int i = 0; i < user->activeOrderCount; i++) {
            int orderID = user->activeOrders[i];
            Order* order = allOrders->get(orderID);
            if (order && order->status == "ACTIVE") {
                orders.push_back(order);
            }
        }
        
        return orders;
    }
    
    // Get user's trade history
    vector<Trade> getUserTrades(string userID) {
        lock_guard<mutex> lock(tradeLock);
        
        vector<Trade> userTrades;
        
        for (const Trade& trade : tradeHistory) {
            if (trade.buyUserID == userID || trade.sellUserID == userID) {
                userTrades.push_back(trade);
            }
        }
        
        return userTrades;
    }
    
    // Print user's complete portfolio
    void printPortfolio(string userID) {
        User* user = users->get(userID);
        if (!user) {
            cout << "User not found\n";
            return;
        }
        
        cout << "\n┌────────────────────────────────────────┐\n";
        cout << "│  PORTFOLIO: " << userID;
        for (int i = userID.length(); i < 27; i++) cout << " ";
        cout << "│\n";
        cout << "├────────────────────────────────────────┤\n";
        
        // Cash balance
        cout << "│  Cash Balance: $" << fixed << setprecision(2) << user->cashBalance;
        for (int i = to_string((int)user->cashBalance).length(); i < 20; i++) cout << " ";
        cout << "│\n";
        
        cout << "├────────────────────────────────────────┤\n";
        cout << "│  Holdings:                             │\n";
        
        if (user->holdingsCount == 0) {
            cout << "│    (no holdings)                       │\n";
        } else {
            for (int i = 0; i < user->holdingsCount; i++) {
                cout << "│    " << user->symbols[i] << ": " << user->quantities[i] << " shares";
                int padding = 31 - user->symbols[i].length() - to_string(user->quantities[i]).length();
                for (int j = 0; j < padding; j++) cout << " ";
                cout << "│\n";
            }
        }
        
        cout << "├────────────────────────────────────────┤\n";
        
        // Active orders
        vector<Order*> activeOrders = getActiveOrders(userID);
        cout << "│  Active Orders: " << activeOrders.size();
        for (int i = to_string(activeOrders.size()).length(); i < 23; i++) cout << " ";
        cout << "│\n";
        
        // Total trades
        vector<Trade> trades = getUserTrades(userID);
        cout << "│  Total Trades: " << trades.size();
        for (int i = to_string(trades.size()).length(); i < 24; i++) cout << " ";
        cout << "│\n";
        
        cout << "└────────────────────────────────────────┘\n";
    }
};

