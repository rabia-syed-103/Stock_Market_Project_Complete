#ifndef PERSISTENT_MATCHING_ENGINE_H
#define PERSISTENT_MATCHING_ENGINE_H

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <memory>
#include "../cache/Cache.h"
#include "../core/User.h"
#include "../core/Order.h"
#include "../core/Trade.h"
#include "../storage/OrderStorage.h"
#include "../storage/UserStorage.h"
#include "../storage/TradeStorage.h"
#include "../storage/MetadataStorage.h"
#include "../storage/SymbolStorage.h"
#include "OrderBook.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

class PersistentMatchingEngine {
private:
    // STORAGE MANAGERS (disk = source of truth)
    OrderStorage orderStorage;
    UserStorage userStorage;
    TradeStorage tradeStorage;
    MetadataStorage metadataStorage;
    SymbolStorage symbolStorage;

    // IN-MEMORY CACHES (for performance)
    LRUCache<int, Order> orderCache;      // Cache 1000 recent orders
    LRUCache<string, User> userCache;     // Cache 100 active users
    
    // ✅ CHANGED: Use map instead of cache for order books
    std::map<std::string, std::shared_ptr<OrderBook>> orderBooks;  // Persistent!

    // LOCKS
    mutex orderLock;
    mutex userLock;
    mutex tradeLock;
    mutex bookLock;

    int nextOrderID;
    int nextTradeID;

public:
    PersistentMatchingEngine() 
        : orderCache(1000), userCache(100) {  // ✅ Removed bookCache
        
        Metadata meta = metadataStorage.loadMetadata();
        nextOrderID = meta.nextOrderID;
        nextTradeID = meta.nextTradeID;
        
        cout << "Initialized PersistentMatchingEngine:\n";
        cout << "  NextOrderID: " << nextOrderID << "\n";
        cout << "  NextTradeID: " << nextTradeID << "\n";

        rebuildAllOrderBooks();
    }

    ~PersistentMatchingEngine() {
        // Save metadata on shutdown
        Metadata meta;
        meta.nextOrderID = nextOrderID;
        meta.nextTradeID = nextTradeID;
        meta.lastSaveTime = time(nullptr);
        metadataStorage.saveMetadata(meta);
        
        cout << "Cache hit rates:\n";
        cout << "  Orders: " << (orderCache.getHitRate() * 100) << "%\n";
        cout << "  Users: " << (userCache.getHitRate() * 100) << "%\n";
        
        // ✅ OrderBooks will be automatically cleaned up by shared_ptr
    }

    void createUser(const std::string& userID, double initialCash) {
        lock_guard<std::mutex> lock(userLock);
        
        // Check if user already exists ON DISK
        if (userStorage.userExists(userID)) {
            std::cout << "User " << userID << " already exists\n";
            return;
        }
        
        // Create user and write to disk immediately
        User user(userID, initialCash);
        userStorage.persist(user);
        
        // Add to cache for fast access
        userCache.put(userID, std::make_shared<User>(user));
        
        std::cout << "Created user " << userID << " with $" << initialCash << "\n";
    }

    User* getUser(const std::string& userID) {
        std::lock_guard<std::mutex> lock(userLock);
        
        // Try cache first
        auto cached = userCache.get(userID);
        if (cached) {
            return cached.get();
        }
        
        // Cache miss - load from disk
        User user = userStorage.loadUser(userID);
        if (user.getUserID().empty()) {
            return nullptr; // User doesn't exist
        }
        
        // Put in cache and return
        auto ptr = std::make_shared<User>(user);
        userCache.put(userID, ptr);
        return ptr.get();
    }

    Order* placeOrder(const std::string& userID, const std::string& symbol,
                     const std::string& side, double price, int quantity) {
        
        // Step 1: Validate stock exists
        if (!symbolExists(symbol)) {
            std::cout << "Error: Stock " << symbol << " does not exist\n";
            return nullptr;
        }

        // Step 2: Load user from disk (or cache)
        User* user = getUser(userID);
        if (!user) {
            std::cout << "Error: User " << userID << " not found\n";
            return nullptr;
        }

        // Step 3: Reserve resources
        {
            std::lock_guard<std::mutex> lock(userLock);
            
            if (side == "BUY") {
                double cost = price * quantity;
                if (!user->deductCash(cost)) {
                    std::cout << "Error: Insufficient funds\n";
                    return nullptr;
                }
            } else { // SELL
                if (user->getStockQuantity(symbol) < quantity) {
                    std::cout << "Error: Insufficient shares\n";
                    return nullptr;
                }
                user->removeStock(symbol, quantity);
            }
            
            // Write updated user to disk immediately
            userStorage.updateUser(*user);
        }

        // Step 4: Create order and persist to disk
        Order* order;
        {
            std::lock_guard<std::mutex> lock(orderLock);
            
            int orderID = nextOrderID++;
            order = new Order(orderID, userID, symbol, side, price, quantity);
            
            // Write order to disk BEFORE matching
            orderStorage.persist(*order);
            
            // Add to cache
            orderCache.put(orderID, std::make_shared<Order>(*order));
            
            // Update metadata periodically
            Metadata meta;
            meta.nextOrderID = nextOrderID;
            meta.nextTradeID = nextTradeID;
            metadataStorage.saveMetadata(meta);

            user->addActiveOrder(order->getOrderID());
            userStorage.updateUser(*user);

        }
     
        
        // Step 5: Match order in order book
        std::cerr << "[DBG] placeOrder: Getting order book\n";
        OrderBook* book = getOrCreateOrderBook(symbol);

        std::cerr << "[DBG] placeOrder: About to call addOrder\n";
        std::vector<Trade> trades = book->addOrder(order);

        std::cerr << "[DBG] placeOrder: addOrder returned, trades.size()=" << trades.size() << "\n";
        cout << "Reached Here After order added!";
        std::cerr << "[DBG] placeOrder: About to process trades\n";

        // Step 6: Process trades
        processTrades(trades);

        std::cerr << "[DBG] placeOrder: Trades processed\n";


        std::cout << "Order placed: " << order->toString() << "\n";
        

        return order;
    }

bool cancelOrder(int orderID, const std::string& userID) {
    std::lock_guard<std::mutex> lock(orderLock);

    DiskOffset off = orderStorage.getOffsetForOrder(orderID);
    Order order = orderStorage.load(off);

    if (order.getOrderID() == 0) {
        cout << "Error: Order not found\n";
        return 0;
    }

    if (order.userID != userID) {
        cout << "Error: Order does not belong to user\n";
        return 0;
    }

    // ✅ CAPTURE remaining BEFORE cancel
    int remaining = order.getRemainingQuantity();

    // Cancel inside order book (this will zero remaining)
    OrderBook* book = getOrCreateOrderBook(order.getSymbol());
    book->cancelOrder(orderID);

    // Refund remaining
    if (remaining > 0) {
        User* user = getUser(userID);
        if (user) {
            if (order.side == "BUY")
                user->addCash(order.price * remaining);
            else
                user->addStock(order.getSymbol(), remaining);

            userStorage.updateUser(*user);
        }
    }

    // ✅ REMOVE FROM USER ACTIVE ORDERS
    User* user = getUser(userID);
    if (user) {
        user->removeActiveOrder(orderID);
        userStorage.updateUser(*user);
    }

    // Mark cancelled persistently
    Order cancelled = orderStorage.load(off);
    cancelled.status = "CANCELLED";
    orderStorage.save(cancelled, off);

    cout << "Order " << orderID
         << " cancelled, refunded "
         << remaining << " units\n";
    return true;
}


    bool addStock(const std::string& symbol,
              const std::string& userID,
              int initialQuantity) {

        // 🔒 Admin-only
        if (userID != "admin123") {
            std::cout << "Unauthorized: only admin can add stocks\n";
            return false;
        }

        if (symbol.empty() || initialQuantity <= 0) {
            std::cout << "Invalid stock symbol or quantity\n";
            return false;
        }

        // Prevent duplicates
        if (symbolExists(symbol)) {
            std::cout << "Stock already exists: " << symbol << "\n";
            return false;
        }

        // 1️⃣ Persist symbol (disk = source of truth)
        symbolStorage.addSymbol(symbol);

        // 2️⃣ Load admin user
        User* admin = getUser(userID);
        if (!admin) {
            std::cout << "Admin user not found\n";
            return false;
        }

        // 3️⃣ Credit initial shares to admin
        {
            std::lock_guard<std::mutex> lock(userLock);
            admin->addStock(symbol, initialQuantity);
            userStorage.updateUser(*admin);
        }

        std::cout << "Stock " << symbol
                << " added with "
                << initialQuantity
                << " shares issued to admin\n";

        return true;
    }


    bool symbolExists(const std::string& symbol) {
        std::vector<std::string> symbols = symbolStorage.loadAllSymbols();
        return std::find(symbols.begin(), symbols.end(), symbol) != symbols.end();
    }

    vector<string> getAllSymbols() {
        return symbolStorage.loadAllSymbols();
    }
   
    std::vector<Trade> getUserTrades(const std::string& userID) {
        // Load all trades from disk and filter
        std::vector<Trade> allTrades = tradeStorage.loadAllTrades();
        std::vector<Trade> userTrades;
        
        for (const Trade& t : allTrades) {
            if (t.buyUserID == userID || t.sellUserID == userID) {
                userTrades.push_back(t);
            }
        }
        
        return userTrades;
    }

    json getAllUserIDsJSON(const std::string& requestingUserID) {
        json result = json::array();

        // 🔒 Admin-only access
        if (requestingUserID != "admin123") {
            std::cout << "Unauthorized access to user IDs\n";
            return result;
        }

        std::vector<User> users = userStorage.loadAllUsers();

        // Return only user IDs (no cache pollution)
        for (const auto& u : users) {
            if(u.getUserID() == "admin123") continue; 
            result.push_back(u.getUserID());
        }

        return result;
    }



    std::vector<Order*> getActiveOrders(const std::string& userID) {
        // Load user from disk
        User* user = getUser(userID);
        if (!user) return {};
        
        std::vector<Order*> orders;
        std::vector<int> orderIDs = user->getActiveOrderIDs();
        
        for (int id : orderIDs) {
            Order* order = getOrderFromDisk(id);
            if (order) orders.push_back(order);
        }
        
        return orders;
    }

    void printPortfolio(const std::string& userID) {
        User* user = getUser(userID);
        if (!user) {
            std::cout << "User not found\n";
            return;
        }
        
        std::cout << "\n=== Portfolio: " << userID << " ===\n";
        std::cout << "Cash: $" << user->getCashBalance() << "\n";
        
        auto holdings = user->getAllHoldings();
        std::cout << "Holdings:\n";
        for (const auto& h : holdings) {
            std::cout << "  " << h.symbol << ": " << h.quantity << " shares\n";
        }
    }

    void printOrderBook(const string& symbol)
    {
        OrderBook* book = getOrCreateOrderBook(symbol);
        book->printOrderBook();
    }

    json getOrderBook(const std::string& symbol) {
        OrderBook* book = getOrCreateOrderBook(symbol);
        if (!book) {
            return { {"error", "OrderBook not found"} };
        }
        return book->getOrderBook();
    }


    json getPortfolio(const std::string& userID) {
        json result;

        User* user = getUser(userID);
        if (!user) {
            result["error"] = "User not found";
            return result;
        }

        // Basic info
        result["userID"] = userID;

        double reservedCash = 0;

        // Cash section
        result["cash"] = {
            {"available", user->getCashBalance()},

        };

        // Holdings section
        json holdingsJson = json::object();

        for (const auto& h : user->getAllHoldings()) {
            holdingsJson[h.symbol] = {
                {"total",     h.quantity},
            };
        }

        result["holdings"] = holdingsJson;
        // Active orders section
        result["activeOrders"] = json::array();
        vector<Order*> orders = getActiveOrders(userID);
        if(orders.empty()) {
            cout << "No active orders for user " << userID << "\n";
        }
        for (Order* order : orders) {
            result["activeOrders"].push_back({
                {"orderID",   order->orderID},
                {"symbol",    order->symbol},
                {"side",      order->side},
                {"price",     order->price},
                {"quantity",  order->quantity},
                {"remaining", order->remainingQty},
                {"status",    order->status}
            });

            if (order->side == "BUY") {
                reservedCash += order->price * order->getRemainingQuantity();
            }
        }


        return result;
    }

private:
 
    
OrderBook* getOrCreateOrderBook(const std::string& symbol) {
        std::lock_guard<std::mutex> lock(bookLock);
        
        // Check if already exists
        auto it = orderBooks.find(symbol);
        if (it != orderBooks.end()) {
            return it->second.get();
        }
        
        // Create new order book (persists forever until shutdown)
        auto book = std::make_shared<OrderBook>(symbol, orderStorage);
        orderBooks[symbol] = book;
        
        std::cerr << "[INFO] Created order book for " << symbol << "\n";
        
        return book.get();
    }

    Order loadOrderFromDisk(int orderID) {
        // Try cache first
        auto cached = orderCache.get(orderID);
        if (cached) {
            return *cached;
        }
        
        // Load from disk
        DiskOffset offset = orderStorage.getOffsetForOrder(orderID);
        if (offset) {
            Order order = orderStorage.load(offset);
            orderCache.put(orderID, std::make_shared<Order>(order));
            return order;
        }
        
        return Order(); // Empty order if not found
    }

    Order* getOrderFromDisk(int orderID) {
        auto cached = orderCache.get(orderID);
        if (cached) return cached.get();
        
        DiskOffset offset = orderStorage.getOffsetForOrder(orderID);
        if (!offset) return nullptr;
        
        Order order = orderStorage.load(offset);
        auto ptr = std::make_shared<Order>(order);
        orderCache.put(orderID, ptr);
        return ptr.get();
    }

void processTrades(const std::vector<Trade>& trades) {
    for (Trade trade : trades) {
        {
            std::lock_guard<std::mutex> lock(tradeLock);
            trade.tradeID = nextTradeID++;
        }
        
        // Handle price refunds for buyers
        Order* buyOrder = getOrderFromDisk(trade.buyOrderID);
        if (buyOrder && buyOrder->side == "BUY") {
            double reservedPrice = buyOrder->price;
            double executedPrice = trade.price;
            
            if (executedPrice < reservedPrice) {
                double refund = (reservedPrice - executedPrice) * trade.quantity;
                
                User* buyer = getUser(trade.buyUserID);
                if (buyer) {
                    std::lock_guard<std::mutex> lock(userLock);
                    buyer->addCash(refund);
                    userStorage.updateUser(*buyer);
                    std::cout << "Refunded $" << refund << " to " << trade.buyUserID 
                              << " (price diff: " << reservedPrice << " vs " << executedPrice << ")\n";
                }
            }
        }
        
        // Update users' portfolios
        updateUsersForTrade(trade);
        
        // ✅ NEW: Update both orders on disk and clean up active orders
        updateOrdersAfterTrade(trade);
        
        // Persist trade to disk immediately
        tradeStorage.persist(trade);
        
        std::cout << "Trade executed: " << trade.toString() << "\n";
    }
}

// ✅ NEW FUNCTION: Update orders after trade execution
void updateOrdersAfterTrade(const Trade& trade) {
    std::lock_guard<std::mutex> lock(orderLock);
    
    // Update buy order
    DiskOffset buyOffset = orderStorage.getOffsetForOrder(trade.buyOrderID);
    if (buyOffset) {
        Order buyOrder = orderStorage.load(buyOffset);
        
        // If fully filled, mark as FILLED and remove from active orders
        if (buyOrder.remainingQty == 0) {
            buyOrder.status = "FILLED";
            orderStorage.save(buyOrder, buyOffset);
            
            User* buyer = getUser(trade.buyUserID);
            if (buyer) {
                std::lock_guard<std::mutex> lock(userLock);
                buyer->removeActiveOrder(trade.buyOrderID);
                userStorage.updateUser(*buyer);
                std::cout << "Removed filled order " << trade.buyOrderID 
                          << " from " << trade.buyUserID << "'s active orders\n";
            }
        } else {
            // Partial fill - just update the order on disk
            orderStorage.save(buyOrder, buyOffset);
        }
        
        // Update cache
        orderCache.put(trade.buyOrderID, std::make_shared<Order>(buyOrder));
    }
    
    // Update sell order
    DiskOffset sellOffset = orderStorage.getOffsetForOrder(trade.sellOrderID);
    if (sellOffset) {
        Order sellOrder = orderStorage.load(sellOffset);
        
        // If fully filled, mark as FILLED and remove from active orders
        if (sellOrder.remainingQty == 0) {
            sellOrder.status = "FILLED";
            orderStorage.save(sellOrder, sellOffset);
            
            User* seller = getUser(trade.sellUserID);
            if (seller) {
                std::lock_guard<std::mutex> lock(userLock);
                seller->removeActiveOrder(trade.sellOrderID);
                userStorage.updateUser(*seller);
                std::cout << "Removed filled order " << trade.sellOrderID 
                          << " from " << trade.sellUserID << "'s active orders\n";
            }
        } else {
            // Partial fill - just update the order on disk
            orderStorage.save(sellOrder, sellOffset);
        }
        
        // Update cache
        orderCache.put(trade.sellOrderID, std::make_shared<Order>(sellOrder));
    }
}


    void updateUsersForTrade(const Trade& trade) {
    // NO LOCK HERE - getUser() will lock internally
    
    User* buyer = getUser(trade.buyUserID);
    User* seller = getUser(trade.sellUserID);
    
    if (!buyer || !seller) return;
    
    // Lock only when modifying
    {
        std::lock_guard<std::mutex> lock(userLock);
        buyer->addStock(trade.symbol, trade.quantity);
        seller->addCash(trade.price * trade.quantity);
        
        userStorage.updateUser(*buyer);
        userStorage.updateUser(*seller);
    }
}

    void refundUser(const std::string& userID, const Order& order) {
        //std::lock_guard<std::mutex> lock(userLock);
        
        User* user = getUser(userID);
        if (!user) return;
        
        int remaining = order.getRemainingQuantity();
        if (remaining > 0) {
            if (order.side == "BUY") {
                user->addCash(order.price * remaining);
            } else {
                user->addStock(order.getSymbol(), remaining);
            }
            
            userStorage.updateUser(*user);
        }
    }

    void rebuildAllOrderBooks() {
        vector<string> symbols = symbolStorage.loadAllSymbols();
        
        for (const string& symbol : symbols) {
            auto book = std::make_shared<OrderBook>(symbol, orderStorage);
            book->rebuildFromStorage();
            orderBooks[symbol] = book;  // ✅ Store in map, not cache
        }
        
        cout << "Rebuilt " << symbols.size() << " order books from storage.\n";
    }
};


#endif // PERSISTENT_MATCHING_ENGINE_H