#ifndef MATCHINGENGINE_H
#define MATCHINGENGINE_H

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
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
    MatchingEngine() : nextOrderID(1), nextTradeID(1) {
        orderBooks = new MyHashMap<string, OrderBook*>(100);
        allOrders = new MyHashMap<int, Order*>(10000);
        users = new MyHashMap<string, User*>(1000);
        
        orderBooks->insert("AAPL", new OrderBook("AAPL"));
        orderBooks->insert("GOOGL", new OrderBook("GOOGL"));
        orderBooks->insert("TSLA", new OrderBook("TSLA"));
        orderBooks->insert("MSFT", new OrderBook("MSFT"));
    }
    
    ~MatchingEngine() {
        // TODO: Properly cleanup all allocated objects
        // This is simplified - in production you'd iterate and delete
        delete orderBooks;
        delete allOrders;
        delete users;
    }
    
    void createUser(string userID, double initialCash) {
        lock_guard<mutex> lock(userLock);
        
        // Check if user exists
        if (users->contains(userID)) {
            cout << "User " << userID << " already exists\n";
            return;
        }
        
        User* user = new User(userID, initialCash);
        users->insert(userID, user);
        
        cout << "Created user " << userID << " with $" << initialCash << "\n";
    }
    
    User* getUser(string userID) {
        lock_guard<mutex> lock(userLock);
        
        if (!users->contains(userID)) {
            return nullptr;
        }
        
        return users->get(userID);
    }
  
    Order* placeOrder(string userID, string symbol, 
                      string side, double price, int quantity) {
        
        cout <<"No issue";
        // Step 1: Validate user and resources
        {
            lock_guard<mutex> lock(userLock);
            
            if (!users->contains(userID)) {
                cout << "Error: User " << userID << " not found\n";
                return nullptr;
            }
            
            User* user = users->get(userID);
            
            if (side == "BUY") {
                double cost = price * quantity;
                cout <<"No Issue 2";
                if (!user->deductCash(cost)) {
                    cout << "Error: Insufficient funds. Need $" << cost 
                         << ", have $" << user->getCashBalance() << "\n";
                    return nullptr;
                }
                
            } else {  // SELL
                // Check if user has enough shares
                if (user->getStockQuantity(symbol) < quantity) {
                    cout << "Error: Insufficient shares of " << symbol << "\n";
                    return nullptr;
                }
                
                // Remove shares immediately (they're locked for this order)
                if (!user->removeStock(symbol, quantity)) {
                    cout << "Error: Failed to lock shares for sell order\n";
                    return nullptr;
                }
            }
        }
        
        // Step 2: Create order
        // Step 1: Validate user and reserve resources + allocate order ID and register order atomically
        Order* order = nullptr;
        {
            // Acquire engineLock first, then userLock to preserve consistent ordering
            std::scoped_lock lock(engineLock, userLock);

            if (!users->contains(userID)) {
                cout << "Error: User " << userID << " not found\n";
                return nullptr;
            }

            User* user = users->get(userID);

            if (side == "BUY") {
                double cost = price * quantity;
                if (!user->deductCash(cost)) {
                    cout << "Error: Insufficient funds. Need $" << cost 
                        << ", have $" << user->getCashBalance() << "\n";
                    return nullptr;
                }
            } else { // SELL
                if (user->getStockQuantity(symbol) < quantity) {
                    cout << "Error: Insufficient shares of " << symbol << "\n";
                    return nullptr;
                }
                if (!user->removeStock(symbol, quantity)) {
                    cout << "Error: Failed to lock shares for sell order\n";
                    return nullptr;
                }
            }

            // allocate order id and create order under engineLock (already held)
            int orderID = nextOrderID++;
            order = new Order(orderID, userID, symbol, side, price, quantity);
            allOrders->insert(orderID, order);

            // register active order for user (we already hold userLock)
            user->addActiveOrder(orderID);
        }

        // Step 2: Get order book (engineLock only)
        OrderBook* book;
        {
            lock_guard<mutex> lock(engineLock);
            if (!orderBooks->contains(symbol)) {
                book = new OrderBook(symbol);
                orderBooks->insert(symbol, book);
            } else {
                book = orderBooks->get(symbol);
            }
        }

        // Step 3: Add to orderbook and perform matching (book->addOrder may return trades)
        vector<Trade> trades = book->addOrder(order);

   
        
        // Step 6: Process trades
        for (Trade& trade : trades) {
            {
                lock_guard<mutex> lock(engineLock);
                trade.tradeID = nextTradeID++;
            }
            
            {
                lock_guard<mutex> lock(userLock);
                
                if (!users->contains(trade.buyUserID) || !users->contains(trade.sellUserID)) {
                    continue;
                }
                
                User* buyer = users->get(trade.buyUserID);
                User* seller = users->get(trade.sellUserID);
                
                // For BUY orders: shares were already removed from seller in their SELL order
                // Now transfer to buyer
                buyer->addStock(symbol, trade.quantity);
                
                // For SELL orders: shares were removed when order was placed
                // Now just transfer cash to seller
                seller->addCash(trade.price * trade.quantity);
                
                cout << "Trade executed: " << trade.toString() << "\n";
            }
            
            {
                lock_guard<mutex> lock(tradeLock);
                tradeHistory.push_back(trade);
            }
        }
        
        // Step 7: Handle unfilled portion
        if (order->status != "FILLED") {
            lock_guard<mutex> lock(userLock);
            
            if (!users->contains(userID)) {
                return order;
            }
            
            User* user = users->get(userID);
            
            if (side == "BUY") {
                // Refund locked cash for unfilled BUY orders
                user->addCash(price * order->getRemainingQuantity());
            } else {
                // Return unsold shares
                user->addStock(symbol, order->getRemainingQuantity());
            }
        }
        
        // Step 8: Remove from active orders if filled
        if (order->isFilled()) {
            lock_guard<mutex> lock(userLock);
            if (users->contains(userID)) {
                User* user = users->get(userID);
                user->removeActiveOrder(order->getOrderID());
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
            
            if (!allOrders->contains(orderID)) {
                cout << "Error: Order " << orderID << " not found\n";
                return;
            }
            
            order = allOrders->get(orderID);
            
            if (order->userID != userID) {
                cout << "Error: Order " << orderID << " does not belong to " << userID << "\n";
                return;
            }
            
            symbol = order->getSymbol();
            side = order->side;
            remaining = order->getRemainingQuantity();
            price = order->getPrice();
        }
        
        // Cancel in order book
        {
            lock_guard<mutex> lock(engineLock);
            
            if (orderBooks->contains(symbol)) {
                OrderBook* book = orderBooks->get(symbol);
                book->cancelOrder(orderID);
            }
        }
        
        // Refund user
   
        {
            std::scoped_lock lock(engineLock, userLock);

            if (!users->contains(userID)) {
                return;
            }

            User* user = users->get(userID);

            if (remaining > 0) {
                if (side == "BUY") {
                    user->addCash(price * remaining);
                } else {
                    user->addStock(symbol, remaining);
                }
            }

            user->removeActiveOrder(orderID);
        }

    }
    
    vector<Trade> getAllTrades() {
        lock_guard<mutex> lock(tradeLock);
        return tradeHistory;
    }
    
    Order* getOrder(int orderID) {
        lock_guard<mutex> lock(engineLock);
        
        if (!allOrders->contains(orderID)) {
            return nullptr;
        }
        
        return allOrders->get(orderID);
    }
    
    OrderBook* getOrderBook(string symbol) {
        lock_guard<mutex> lock(engineLock);
        
        if (!orderBooks->contains(symbol)) {
            return nullptr;
        }
        
        return orderBooks->get(symbol);
    }
    
    void printOrderBook(const string& symbol) {
        lock_guard<mutex> lock(engineLock);
        
        if (!orderBooks->contains(symbol)) {
            cout << "No order book for symbol: " << symbol << "\n";
            return;
        }
        
        OrderBook* book = orderBooks->get(symbol);
        book->printOrderBook();
    }

    string getPortfolio(string userID) {
        lock_guard<mutex> lock(userLock);
        
        if (!users->contains(userID)) {
            return "User not found";
        }
        
        User* user = users->get(userID);
        return user->toString();
    }
    
    double getCashBalance(string userID) {
        lock_guard<mutex> lock(userLock);
        
        if (!users->contains(userID)) {
            return -1;
        }
        
        User* user = users->get(userID);
        return user->getCashBalance();
    }
    

    
    vector<StockHolding> getHoldings(string userID) {
        lock_guard<mutex> lock(userLock);
        
        vector<StockHolding> holdings;
        
        if (!users->contains(userID)) {
            return holdings;
        }
        
        User* user = users->get(userID);

        holdings = user->getAllHoldings();
        
        return holdings;
    }
    
    vector<Order*> getActiveOrders(string userID) {
        vector<Order*> orders;

        // Acquire engineLock then userLock to follow the global order
        std::scoped_lock lock(engineLock, userLock);

        if (!users->contains(userID)) return orders;

        User* user = users->get(userID);
        vector<int> ids = user->getActiveOrderIDs();
        for (int id : ids) {
            if (allOrders->contains(id)) {
                orders.push_back(allOrders->get(id));
            }
        }

        return orders;
    }

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
    void printPortfolio(string userID) {
        User* user = getUser(userID);
        if (!user) {
            cout << "User not found\n";
            return;
        }
        
        // Get all data
        double balance = user->getCashBalance();
        vector<StockHolding> holdings = user->getAllHoldings();
        vector<Order*> activeOrders = getActiveOrders(userID);
        vector<Trade> trades = getUserTrades(userID);
        
        cout << "\n┌────────────────────────────────────────┐\n";
        cout << "│  PORTFOLIO: " << userID;
        for (size_t i = userID.length(); i < 27; i++) cout << " ";
        cout << "│\n";
        cout << "├────────────────────────────────────────┤\n";
        
        // Cash balance
        cout << "│  Cash Balance: $" << fixed << setprecision(2) << balance;
        string balStr = to_string((int)balance);
        for (size_t i = balStr.length() + 4; i < 28; i++) cout << " ";
        cout << "│\n";
        cout << "├────────────────────────────────────────┤\n";
        
        // Holdings
        cout << "│  Holdings:                             │\n";
        if (holdings.empty()) {
            cout << "│    No stocks owned                     │\n";
        } else {
            for (const auto& holding : holdings) {
                cout << "│    " << holding.symbol << ": " << holding.quantity << " shares";
                // Calculate padding
                int used = 7 + holding.symbol.length() + to_string(holding.quantity).length();
                for (int i = used; i < 39; i++) cout << " ";
                cout << "│\n";
            }
        }
        cout << "├────────────────────────────────────────┤\n";
        
        // Active orders
        cout << "│  Active Orders: " << activeOrders.size();
        for (size_t i = to_string(activeOrders.size()).length(); i < 23; i++) cout << " ";
        cout << "│\n";
        
        // Total trades
        cout << "│  Total Trades: " << trades.size();
        for (size_t i = to_string(trades.size()).length(); i < 24; i++) cout << " ";
        cout << "│\n";
        cout << "└────────────────────────────────────────┘\n";
    }
};

#endif