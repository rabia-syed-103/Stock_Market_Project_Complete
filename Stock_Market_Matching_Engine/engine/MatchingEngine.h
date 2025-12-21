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
#include "../storage/OrderStorage.h"
#include "../storage/StorageManager.h"
#include "../storage/SymbolStorage.h"
#include "../storage/UserStorage.h"
#include "../storage/TradeStorage.h"
#include "../storage/MetadataStorage.h"

using namespace std;

class MatchingEngine {
private:
    MyHashMap<string, OrderBook*>* orderBooks;
    MyHashMap<int, Order*>* allOrders;
    MyHashMap<string, User*>* users;
    
    vector<Trade> tradeHistory;
    SymbolStorage symbolStorage;

    // ADD THESE THREE NEW STORAGE MANAGERS:
    UserStorage userStorage;
    TradeStorage tradeStorage;
    MetadataStorage metadataStorage;

    mutex engineLock;
    mutex tradeLock;
    mutex userLock;
    

    int nextOrderID;
    int nextTradeID;

    OrderStorage orderStorage;

public:

MatchingEngine() : nextOrderID(1), nextTradeID(1) {
    orderBooks = new MyHashMap<string, OrderBook*>(100);
    allOrders = new MyHashMap<int, Order*>(10000);
    users = new MyHashMap<string, User*>(1000);

    Metadata meta = metadataStorage.loadMetadata();
    nextOrderID = meta.nextOrderID;
    nextTradeID = meta.nextTradeID;
    
    cout << "Restored IDs: nextOrderID=" << nextOrderID 
         << ", nextTradeID=" << nextTradeID << "\n";
    
    rebuildAllFromStorage();
    
}

~MatchingEngine() {
    Metadata meta;
    meta.nextOrderID = nextOrderID;
    meta.nextTradeID = nextTradeID;
    meta.totalUsers = users->getSize();
    meta.totalOrders = allOrders->getSize();
    meta.totalTrades = tradeHistory.size();
    meta.lastSaveTime = time(nullptr);
    metadataStorage.saveMetadata(meta);
    // Delete OrderBook*
    std::vector<std::string> symbols = orderBooks->getAllKeys();
    for (const std::string& sym : symbols) {
        delete orderBooks->get(sym);
    }

    // Delete Order*
    std::vector<int> orderIDs = allOrders->getAllKeys();
    for (int id : orderIDs) {
        delete allOrders->get(id);
    }

    // Delete User*
    std::vector<std::string> userIDs = users->getAllKeys();
    for (const std::string& uid : userIDs) {
        delete users->get(uid);
    }

    // Delete the hashmaps themselves
    delete orderBooks;
    delete allOrders;
    delete users;
}


void createUser(string userID, double initialCash) {
    lock_guard<mutex> lock(userLock);
    
    if (users->contains(userID)) {
        cout << "User " << userID << " already exists\n";
        return;
    }
    
    User* user = new User(userID, initialCash);
    users->insert(userID, user);
    
    // PERSIST USER IMMEDIATELY
    userStorage.persist(*user);
    
    cout << "Created user " << userID << " with $" << initialCash << "\n";
}

User* getUser(string userID) {
    lock_guard<mutex> lock(userLock);
    
    if (!users->contains(userID)) {
        return nullptr;
    }
    
    return users->get(userID);
}

Order* placeOrder(
    string userID, string symbol,
    string side, double price, int quantity
) {
    // Step 0: Validate stock exists
    {
        lock_guard<mutex> lock(engineLock);
        if (!orderBooks->contains(symbol)) {
            cout << "NO SUCH STOCK EXISTS\n";
            return nullptr;
        }
    }

    // Step 1: Validate user & reserve resources
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
            user->removeStock(symbol, quantity); // lock shares
        }

        // PERSIST USER AFTER RESOURCE DEDUCTION
        userStorage.updateUser(*user);

        int orderID = nextOrderID++;
        order = new Order(orderID, userID, symbol, side, price, quantity);
        allOrders->insert(orderID, order);
        user->addActiveOrder(orderID);
        
        // PERSIST USER AFTER ADDING ACTIVE ORDER
        userStorage.updateUser(*user);
    }

    // Step 2: Get order book
    OrderBook* book;
    {
        lock_guard<mutex> lock(engineLock);
        book = orderBooks->get(symbol);
    }

    // Step 3: Add to order book (returns trades)
    vector<Trade> trades = book->addOrder(order);

    // Step 3.5: Reload the incoming order from disk to get updated status
    {
        lock_guard<mutex> lock(engineLock);
        DiskOffset off = orderStorage.getOffsetForOrder(order->getOrderID());
        if (off) {
            Order diskOrder = orderStorage.load(off);
            if (allOrders->contains(order->getOrderID())) {
                *allOrders->get(order->getOrderID()) = diskOrder;
                order = allOrders->get(order->getOrderID());
            }
        }
    }

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

            // transfer assets/cash
            buyer->addStock(trade.symbol, trade.quantity);
            seller->addCash(trade.price * trade.quantity);

            // PERSIST BOTH USERS AFTER TRADE EXECUTION
            userStorage.updateUser(*buyer);
            userStorage.updateUser(*seller);

            cout << "Trade executed: " << trade.toString() << "\n";
        }

        // Sync matched orders from disk and remove filled active-orders
        {
            lock_guard<mutex> lock(engineLock);
            int buyID = trade.buyOrderID;
            int sellID = trade.sellOrderID;

            // Load and sync buy order from disk
            DiskOffset buyOff = orderStorage.getOffsetForOrder(buyID);
            if (buyOff && allOrders->contains(buyID)) {
                Order diskBuy = orderStorage.load(buyOff);
                *allOrders->get(buyID) = diskBuy;

                if (diskBuy.isFilled()) {
                    lock_guard<mutex> ulock(userLock);
                    if (users->contains(diskBuy.userID)) {
                        users->get(diskBuy.userID)->removeActiveOrder(buyID);
                        // PERSIST USER AFTER REMOVING ACTIVE ORDER
                        userStorage.updateUser(*users->get(diskBuy.userID));
                    }
                }
            }

            // Load and sync sell order from disk
            DiskOffset sellOff = orderStorage.getOffsetForOrder(sellID);
            if (sellOff && allOrders->contains(sellID)) {
                Order diskSell = orderStorage.load(sellOff);
                *allOrders->get(sellID) = diskSell;

                if (diskSell.isFilled()) {
                    lock_guard<mutex> ulock(userLock);
                    if (users->contains(diskSell.userID)) {
                        users->get(diskSell.userID)->removeActiveOrder(sellID);
                        // PERSIST USER AFTER REMOVING ACTIVE ORDER
                        userStorage.updateUser(*users->get(diskSell.userID));
                    }
                }
            }
        }

        {
            lock_guard<mutex> lock(tradeLock);
            tradeHistory.push_back(trade);
            // PERSIST TRADE IMMEDIATELY
            tradeStorage.persist(trade);
        }
    }

    // Step 5: Remove incoming order from active-orders if it was filled by matches
    if (order->isFilled()) {
        lock_guard<mutex> lock(userLock);
        if (users->contains(order->userID)) {
            users->get(order->userID)->removeActiveOrder(order->getOrderID());
            // PERSIST USER AFTER REMOVING ACTIVE ORDER
            userStorage.updateUser(*users->get(order->userID));
        }
    }

    // PERSIST METADATA PERIODICALLY (every 10 orders)
    if (nextOrderID % 10 == 0) {
        Metadata meta;
        meta.nextOrderID = nextOrderID;
        meta.nextTradeID = nextTradeID;
        meta.totalUsers = users->getSize();
        meta.totalOrders = allOrders->getSize();
        meta.totalTrades = tradeHistory.size();
        meta.lastSaveTime = time(nullptr);
        metadataStorage.saveMetadata(meta);
    }
    
    cout << "Order Status: " << order->toString() << "\n";
    return order;
}

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

    // After refunding user
    {
        lock_guard<mutex> lock(userLock);
        if (users->contains(userID)) {
            userStorage.updateUser(*users->get(userID));
        }
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

// ...existing code...
void rebuildAllFromStorage() {
    // 1️⃣ Load all users FIRST (before orders)
    vector<User> loadedUsers = userStorage.loadAllUsers();
    for (const User& u : loadedUsers) {
        User* userPtr = new User(u);
        users->insert(userPtr->getUserID(), userPtr);
    }
    cout << "Loaded " << loadedUsers.size() << " users from storage.\n";
    
    // 2️⃣ Rebuild all order books from symbol storage
    vector<string> symbols = symbolStorage.loadAllSymbols();
    int restoredOrders = 0;

    for (const string& symbol : symbols) {
        if (!orderBooks->contains(symbol)) {
            orderBooks->insert(symbol, new OrderBook(symbol, orderStorage));
        }

        OrderBook* book = orderBooks->get(symbol);

        // 3️⃣ Load orders for this symbol
        vector<Order> ordersForSymbol = orderStorage.loadAllOrdersForSymbol(symbol);

        for (const Order& order : ordersForSymbol) {
            if (order.status == "FILLED" || order.status == "CANCELLED") continue;

            // Create Order* and add to map
            Order* o = new Order(order);
            allOrders->insert(o->getOrderID(), o);
            restoredOrders++;

            // User should already exist from step 1
            if (users->contains(order.userID)) {
                User* u = users->get(order.userID);
                u->addActiveOrder(o->getOrderID());
            }
        }

        // 4️⃣ Rebuild order book tree from storage
        book->rebuildFromStorage();
    }

    // 5️⃣ Load all trades into tradeHistory
    vector<Trade> loadedTrades = tradeStorage.loadAllTrades();
    tradeHistory = loadedTrades;
    cout << "Loaded " << tradeHistory.size() << " trades from storage.\n";

    cout << "Rebuilt " << symbols.size() << " order books from storage.\n";
    cout << "Restored " << restoredOrders << " active orders from storage.\n";
}

bool addStock(const std::string& symbol, const std::string& userID) {
    {
        std::scoped_lock lock(engineLock);

        if (userID != "admin123") {
            std::cout << "User " << userID << " is not authorized to add stocks\n";
            return false;
        }

        if (orderBooks->contains(symbol)) {
            std::cout << "Stock " << symbol << " already exists\n";
            return false;
        }

        OrderBook* book = new OrderBook(symbol,orderStorage);
        orderBooks->insert(symbol, book);

        std::cout << "Stock " << symbol << " added successfully by " << userID << "\n";
    }

    // Persist symbol outside engineLock to avoid deadlock
    symbolStorage.addSymbol(symbol);

    return true;
}

bool symbolExists(const std::string& symbol) {
    std::scoped_lock lock(engineLock);
    return orderBooks->contains(symbol);
}


};

#endif