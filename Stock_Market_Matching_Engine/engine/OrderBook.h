#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <vector>
#include <pthread.h>  
#include "../data_structures/BTree.h"
#include "../core/Order.h"
#include "../core/Trade.h"
#include <algorithm>  

using namespace std;

class OrderBook {
private:
    string symbol;
    BTree* buyTree;   // Max heap for bids
    BTree* sellTree;  // Min heap for asks
    pthread_mutex_t bookLock;  
    OrderStorage& orderStorage;  // Reference to storage (disk-first)

public:
    OrderBook(std::string sym, OrderStorage& _order);
    ~OrderBook();
    
    // Core operations (thread-safe)
    vector<Trade> addOrder(Order* order);
    void cancelOrder(int orderID);
    
    // Query operations (thread-safe)
    Order getBestBid();
    Order getBestAsk();
    void printOrderBook();
    string getOrderBookJSON();
    string getSymbol() const; 
    
    // NEW: Rebuild from disk on startup
    void rebuildFromStorage();
    
    
private:
    // NEW: Load order from storage (uses cache in MatchingEngine)
    Order loadOrderFromStorage(int orderID);
};

#endif