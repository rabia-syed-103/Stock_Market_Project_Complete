#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <vector>
#include <pthread.h>  
#include "../data_structures/BTree.h"
#include "../core/Order.h"
#include "../core/Trade.h"

class OrderBook {
private:
    std::string symbol;
    BTree* buyTree;   // Max
    BTree* sellTree;  // Min
    
    pthread_mutex_t bookLock;  

public:
    OrderBook(std::string sym);
    ~OrderBook();
    
    // Core operations (thread-safe)
    vector<Trade> addOrder(Order* order);
    void cancelOrder(int orderID);
    
    // Query operations (thread-safe)
    Order* getBestBid();
    Order* getBestAsk();
    void printOrderBook();
    string getOrderBookJSON();
    
    string getSymbol() const; 
};

#endif

