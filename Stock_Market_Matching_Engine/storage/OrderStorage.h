#pragma once

#include "../core/Order.h"
#include "StorageManager.h"
#include <vector>
#include <unordered_map>
#include <mutex>

using DiskOffset = uint64_t;
using namespace std;

class OrderStorage {
private:
    StorageManager storage;
    
    // CHANGED: Only keep indexes, not full orders
    unordered_map<int, DiskOffset> orderIDToOffsetMap;  // orderID -> offset
    
    // NEW: Secondary indexes for fast queries
    unordered_map<string, vector<int>> symbolToOrdersMap;  // symbol -> orderIDs
    unordered_map<string, vector<int>> userToOrdersMap;    // userID -> orderIDs
    
    // REMOVED: ordersMap - data lives on disk
    
    mutable mutex indexMutex;  // NEW: Thread safety

public:
    OrderStorage();
    OrderStorage(const OrderStorage&) = delete;
    OrderStorage& operator=(const OrderStorage&) = delete;
    ~OrderStorage();
    
    DiskOffset persist(const Order& order);
    Order load(DiskOffset offset);
    void save(const Order& order, DiskOffset offset);
    
    // Existing methods
    vector<Order> loadAllOrdersForSymbol(const string& symbol);
    DiskOffset getOffsetForOrder(int orderID);
    vector<Order> loadAllOrders();
    
    // NEW: Query methods for disk-first design
    Order loadOrder(int orderID);
    vector<Order> loadOrdersForUser(const string& userID);
    bool orderExists(int orderID);
    
private:
    // NEW: Index management
    void loadIndex();
    void saveIndex();
    void rebuildIndex();
};