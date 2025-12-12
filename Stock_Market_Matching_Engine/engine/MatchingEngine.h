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
#include <memory>

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

    // Step 1: Validate user and reserve resources + allocate order ID
    Order* order = nullptr;
    {
        scoped_lock lock(engineLock, userLock);

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

        // Allocate order ID and create order
        int orderID = nextOrderID++;
        order = new Order(orderID, userID, symbol, side, price, quantity);
        allOrders->insert(orderID, order);

        // Register active order for user
        user->addActiveOrder(orderID);
    }

    // Step 2: Get order book
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

    // Step 3: Add to orderbook and get trades
    vector<Trade> trades = book->addOrder(order);

    // Step 4: Process trades
    for (Trade& trade : trades) {
        {
            lock_guard<mutex> lock(engineLock);
            trade.tradeID = nextTradeID++;
        }

        {
            lock_guard<mutex> lock(userLock);
            if (!users->contains(trade.buyUserID) || !users->contains(trade.sellUserID))
                continue;

            User* buyer = users->get(trade.buyUserID);
            User* seller = users->get(trade.sellUserID);

            // Transfer shares to buyer (cash already deducted in Step 1)
            buyer->addStock(symbol, trade.quantity);

            // Add cash to seller (shares already removed when they placed SELL order)
            seller->addCash(trade.price * trade.quantity);

            cout << "Trade executed: " << trade.toString() << "\n";
            
            // Update order status
              
            if(order->side == "BUY")
            {
            order->remainingQty -= trade.quantity;
          
            if (order->getRemainingQuantity() == 0)
                order->status = "FILLED";
            else
                order->status = "PARTIAL_FILL";
            }
           // if(order->side == "SELL")

        }

        {
            lock_guard<mutex> lock(tradeLock);
            tradeHistory.push_back(trade);
        }
    }

    // Step 5: Remove from active orders if filled
    if (order->isFilled()) {
        lock_guard<mutex> lock(userLock);
        if (users->contains(userID)) {
            User* user = users->get(userID);
            user->removeActiveOrder(order->getOrderID());
        }
    }

    return order;
}


/*
void cancelOrder(int orderID, const string& userID) {
    Order* order = nullptr;
    
    // Step 1: Lock engine and get the order
    {
        std::scoped_lock lock(engineLock, userLock);

        if (!allOrders->contains(orderID)) {
            std::cout << "Error: Order " << orderID << " not found\n";
            return;
        }

        order = allOrders->get(orderID);

        if (order->userID != userID) {
            std::cout << "Error: Order " << orderID << " does not belong to " << userID << "\n";
            return;
        }
    }

    // Step 2: Cancel in order book (removes from BTree)
    if (orderBooks->contains(order->getSymbol())) {
        OrderBook* book = orderBooks->get(order->getSymbol());
        book->cancelOrder(orderID);
    }

    // Step 3: Refund only remaining unfilled portion
    {
        std::scoped_lock lock(userLock);

        if (!users->contains(userID)) return;

        User* user = users->get(userID);

        int remaining = order->getRemainingQuantity(); // IMPORTANT: remainingQty is always correct
        if (remaining > 0) {
            if (order->side == "BUY") {
                user->addCash(order->price * remaining); // refund exact unfilled cash
            } else { // SELL
                user->addStock(order->getSymbol(), remaining); // return unsold shares
            }
        }

        // Step 4: Remove from active orders
        user->removeActiveOrder(orderID);
    }

    // Step 5: Mark order as cancelled
    order->status = "CANCELLED";
    std::cout << "Cancelled OrderID " << orderID 
              << " from " << order->side << " side, Refund processed.\n";
}
*/

void cancelOrder(int orderID, const string& userID) {
    Order* order = nullptr;
    int remaining = 0;  // Save this early
    string side;
    string symbol;
    double price;
    
    // Step 1: Lock engine and get the order info
    {
        std::scoped_lock lock(engineLock, userLock);

        if (!allOrders->contains(orderID)) {
            std::cout << "Error: Order " << orderID << " not found\n";
            return;
        }

        order = allOrders->get(orderID);

        if (order->userID != userID) {
            std::cout << "Error: Order " << orderID << " does not belong to " << userID << "\n";
            return;
        }
        
        // IMPORTANT: Save values BEFORE cancelling from order book
        remaining = order->getRemainingQuantity();
        side = order->side;
        symbol = order->getSymbol();
        price = order->price;
    }

    // Step 2: Cancel in order book (this might modify the order object)
    if (orderBooks->contains(symbol)) {
        OrderBook* book = orderBooks->get(symbol);
        book->cancelOrder(orderID);
    }

    // Step 3: Refund using saved values
    {
        std::scoped_lock lock(userLock);

        if (!users->contains(userID)) return;

        User* user = users->get(userID);
        
        if (remaining > 0) {
            if (side == "BUY") {
                user->addCash(price * remaining);
            } else { // SELL
                user->addStock(symbol, remaining);
            }
        }

        // Step 4: Remove from active orders
        user->removeActiveOrder(orderID);
    }

    // Step 5: Mark order as cancelled
    order->status = "CANCELLED";
    std::cout << "Cancelled OrderID " << orderID 
              << " from " << side << " side, Refund processed.\n";
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
   
void printPortfolio(const string& userID) {
    // Local copies for printing
    double balance = 0.0;
    vector<StockHolding> holdings;
    vector<int> activeOrderIDs;
    vector<Trade> userTrades;

    // Acquire locks in the global order (engineLock then userLock)
    {
        std::scoped_lock lock(engineLock, userLock);

        // get user pointer directly from users hashmap (we hold userLock)
        if (!users->contains(userID)) {
            cout << "User not found\n";
            return;
        }
        User* user = users->get(userID);

        // Copy user state (these return simple copies)
        balance = user->getCashBalance();
        holdings = user->getAllHoldings();
        activeOrderIDs = user->getActiveOrderIDs();

        // Copy trades while still holding engineLock? tradeHistory is protected by tradeLock,
        // so we can't safely read it here without holding tradeLock. We'll copy trades outside.
    }

    // Copy trades under tradeLock separately (no nested locks with engine/user)
    {
        std::lock_guard<std::mutex> tlock(tradeLock);
        for (const Trade &tr : tradeHistory) {
            if (tr.buyUserID == userID || tr.sellUserID == userID) {
                userTrades.push_back(tr);
            }
        }
    }

    // Now print using the local copies (no locks held)
    cout << "\n┌────────────────────────────────────────┐\n";
    cout << "│  PORTFOLIO: " << userID;
    for (size_t i = userID.length(); i < 27; i++) cout << " ";
    cout << "│\n";
    cout << "├────────────────────────────────────────┤\n";

    cout << "│  Cash Balance: $" << fixed << setprecision(2) << balance;
    string balStr = to_string((int)balance);
    for (size_t i = balStr.length() + 4; i < 28; i++) cout << " ";
    cout << "│\n";
    cout << "├────────────────────────────────────────┤\n";

    cout << "│  Holdings:                             │\n";
    if (holdings.empty()) {
        cout << "│    No stocks owned                     │\n";
    } else {
        for (const auto& holding : holdings) {
            cout << "│    " << holding.symbol << ": " << holding.quantity << " shares";
            int used = 7 + holding.symbol.length() + to_string(holding.quantity).length();
            for (int i = used; i < 39; i++) cout << " ";
            cout << "│\n";
        }
    }
    cout << "├────────────────────────────────────────┤\n";

    cout << "│  Active Orders: " << activeOrderIDs.size();
    for (size_t i = to_string(activeOrderIDs.size()).length(); i < 23; i++) cout << " ";
    cout << "│\n";

    cout << "│  Total Trades: " << userTrades.size();
    for (size_t i = to_string(userTrades.size()).length(); i < 24; i++) cout << " ";
    cout << "│\n";
    cout << "└────────────────────────────────────────┘\n";
}


};

#endif