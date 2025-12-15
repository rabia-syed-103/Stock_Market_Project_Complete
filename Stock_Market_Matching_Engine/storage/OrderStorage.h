#pragma once
#include "../core/Order.h"
#include "StorageManager.h"
#include <vector>
#include <unordered_map>
#include <fstream>

using DiskOffset = uint64_t;
using namespace std;

class OrderStorage {
private:
    StorageManager storage;

    // Maps to track offsets and IDs
    unordered_map<int, DiskOffset> orderIDToOffsetMap;
    unordered_map<DiskOffset, Order> ordersMap;

public:
    OrderStorage();
    OrderStorage(const OrderStorage&) = delete;
    OrderStorage& operator=(const OrderStorage&) = delete;
    ~OrderStorage();

    DiskOffset persist(const Order& order);
    Order load(DiskOffset offset);
    void save(const Order& order, DiskOffset offset);

    vector<Order> loadAllOrdersForSymbol(const std::string& symbol);
    DiskOffset getOffsetForOrder(int orderID);

    vector<Order> loadAllOrders();
};

