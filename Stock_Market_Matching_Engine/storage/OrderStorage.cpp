#include "OrderStorage.h"
#include "../core/Order.h"
#include <fstream>
#include <iostream>

OrderStorage::OrderStorage() : storage("data/orders.dat") {
    // CHANGED: Only load indexes
    loadIndex();
    cout << "Loaded order index: " << orderIDToOffsetMap.size() << " orders.\n";
}

OrderStorage::~OrderStorage() {
    // NEW: Save indexes on shutdown
    saveIndex();
}

DiskOffset OrderStorage::persist(const Order& order) {
    lock_guard<mutex> lock(indexMutex);
    
    OrderRecord rec = order.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(OrderRecord));
    DiskOffset storedOff = rawOff + 1;
    
    // CHANGED: Update indexes only
    orderIDToOffsetMap[order.orderID] = storedOff;
    symbolToOrdersMap[order.symbol].push_back(order.orderID);
    userToOrdersMap[order.userID].push_back(order.orderID);
    
    return storedOff;
}

Order OrderStorage::load(DiskOffset offset) {
    if (offset == 0) {
        std::cerr << "[ERR] load called with offset=0\n";
        return Order();
    }
    
    if (offset == 1) {
        std::cerr << "[ERR] load called with offset=1 (likely invalid)\n";
    }
    
    DiskOffset rawOff = offset - 1;
    
    // ✅ ADD: Check if offset is within file bounds
    std::ifstream checkFile("data/orders.dat", std::ios::binary | std::ios::ate);
    size_t fileSize = checkFile.tellg();
    checkFile.close();
    
    if (rawOff + sizeof(OrderRecord) > fileSize) {
        std::cerr << "[ERR] offset " << offset << " beyond file size " << fileSize << "\n";
        return Order(); // Return empty order
    }
    
    OrderRecord rec;
    storage.read(rawOff, &rec, sizeof(OrderRecord));
    return Order::fromRecord(rec);
}


void OrderStorage::save(const Order& order, DiskOffset offset) {
    if (offset == 0) return;
    
    DiskOffset rawOff = offset - 1;
    OrderRecord rec = order.toRecord();
    storage.write(rawOff, &rec, sizeof(OrderRecord));
    
    // REMOVED: Don't update ordersMap - it doesn't exist
}

DiskOffset OrderStorage::getOffsetForOrder(int orderID) {
    lock_guard<mutex> lock(indexMutex);
    
    auto it = orderIDToOffsetMap.find(orderID);
    return (it != orderIDToOffsetMap.end()) ? it->second : 0;
}

// NEW: Load single order by ID
Order OrderStorage::loadOrder(int orderID) {
    DiskOffset offset = getOffsetForOrder(orderID);
    if (offset == 0) return Order();
    return load(offset);
}

// NEW: Check if order exists
bool OrderStorage::orderExists(int orderID) {
    lock_guard<mutex> lock(indexMutex);
    return orderIDToOffsetMap.find(orderID) != orderIDToOffsetMap.end();
}

// NEW: Load orders for a specific user
vector<Order> OrderStorage::loadOrdersForUser(const string& userID) {
    lock_guard<mutex> lock(indexMutex);
    
    vector<Order> orders;
    auto it = userToOrdersMap.find(userID);
    if (it == userToOrdersMap.end()) return orders;
    
    orders.reserve(it->second.size());
    for (int orderID : it->second) {
        DiskOffset offset = orderIDToOffsetMap[orderID];
        orders.push_back(load(offset));
    }
    
    return orders;
}

vector<Order> OrderStorage::loadAllOrdersForSymbol(const string& symbol) {
    lock_guard<mutex> lock(indexMutex);
    
    vector<Order> orders;
    auto it = symbolToOrdersMap.find(symbol);
    if (it == symbolToOrdersMap.end()) return orders;
    
    orders.reserve(it->second.size());
    for (int orderID : it->second) {
        DiskOffset offset = orderIDToOffsetMap[orderID];
        orders.push_back(load(offset));
    }
    
    return orders;
}

vector<Order> OrderStorage::loadAllOrders() {
    lock_guard<mutex> lock(indexMutex);
    
    vector<Order> result;
    result.reserve(orderIDToOffsetMap.size());
    
    for (const auto& [orderID, offset] : orderIDToOffsetMap) {
        result.push_back(load(offset));
    }
    
    return result;
}

void OrderStorage::loadIndex() {
    ifstream indexFile("data/orders.idx", ios::binary);
    
    if (!indexFile) {
        cout << "Order index not found, rebuilding...\n";
        rebuildIndex();
        return;
    }
    
    // ✅ ADD: Check file size first
    indexFile.seekg(0, ios::end);
    size_t fileSize = indexFile.tellg();
    indexFile.seekg(0, ios::beg);
    
    if (fileSize < sizeof(size_t)) {
        cout << "Order index corrupted, rebuilding...\n";
        indexFile.close();
        rebuildIndex();
        return;
    }
    
    // Load primary index (orderID -> offset)
    size_t count;
    indexFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    // ✅ ADD: Sanity check on count
    if (count > 1000000) {  // More than 1 million orders is suspicious
        cout << "Order index has invalid count: " << count << ", rebuilding...\n";
        indexFile.close();
        rebuildIndex();
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        int orderID;
        indexFile.read(reinterpret_cast<char*>(&orderID), sizeof(orderID));
        
        DiskOffset offset;
        indexFile.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        
        // Read symbol
        size_t symLen;
        indexFile.read(reinterpret_cast<char*>(&symLen), sizeof(symLen));
        
        // ✅ ADD: Validate symbol length
        if (symLen > 100) {  // Symbol longer than 100 chars is invalid
            cout << "Invalid symbol length: " << symLen << ", stopping index load\n";
            indexFile.close();
            rebuildIndex();
            return;
        }
        
        string symbol(symLen, '\0');
        indexFile.read(&symbol[0], symLen);
        
        // Read userID
        size_t userLen;
        indexFile.read(reinterpret_cast<char*>(&userLen), sizeof(userLen));
        
        // ✅ ADD: Validate userID length
        if (userLen > 100) {
            cout << "Invalid userID length: " << userLen << ", stopping index load\n";
            indexFile.close();
            rebuildIndex();
            return;
        }
        
        string userID(userLen, '\0');
        indexFile.read(&userID[0], userLen);
        
        // Build secondary indexes
        orderIDToOffsetMap[orderID] = offset;
        symbolToOrdersMap[symbol].push_back(orderID);
        userToOrdersMap[userID].push_back(orderID);
    }
    
    indexFile.close();
    cout << "Loaded order index: " << orderIDToOffsetMap.size() << " orders.\n";
}


void OrderStorage::saveIndex() {
    ofstream indexFile("data/orders.idx", ios::binary);
    if (!indexFile) {
        cerr << "Failed to save order index\n";
        return;
    }
    
    lock_guard<mutex> lock(indexMutex);
    
    size_t count = orderIDToOffsetMap.size();
    indexFile.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    for (const auto& [orderID, offset] : orderIDToOffsetMap) {
        // Load order to get symbol and userID
        Order order = load(offset);
        
        indexFile.write(reinterpret_cast<const char*>(&orderID), sizeof(orderID));
        indexFile.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
        
        // Write symbol
        size_t symLen = order.symbol.size();
        indexFile.write(reinterpret_cast<const char*>(&symLen), sizeof(symLen));
        indexFile.write(order.symbol.c_str(), symLen);
        
        // Write userID
        size_t userLen = order.userID.size();
        indexFile.write(reinterpret_cast<const char*>(&userLen), sizeof(userLen));
        indexFile.write(order.userID.c_str(), userLen);
    }
    
    indexFile.close();
}


void OrderStorage::rebuildIndex() {
    orderIDToOffsetMap.clear();
    symbolToOrdersMap.clear();
    userToOrdersMap.clear();
    
    const size_t recSize = sizeof(OrderRecord);
    ifstream f("data/orders.dat", ios::binary | ios::ate);
    
    if (!f) {
        cout << "orders.dat not found, starting fresh\n";
        return; // ✅ Exit early if file doesn't exist
    }
    
    size_t fileSize = (size_t)f.tellg();
    f.close();
    
    // ✅ ADD: Exit if file is empty
    if (fileSize == 0) {
        cout << "orders.dat is empty, starting fresh\n";
        return;
    }
    
    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        OrderRecord rec;
        storage.read(rawOff, &rec, recSize);
        
        Order o = Order::fromRecord(rec);
        
        // ✅ ADD: Validate order
        if (o.getOrderID() == 0 || o.symbol[0] == '\0') {
            continue; // Skip invalid orders
        }
        
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1;
        
        orderIDToOffsetMap[o.orderID] = storedOff;
        symbolToOrdersMap[o.symbol].push_back(o.orderID);
        userToOrdersMap[o.userID].push_back(o.orderID);
    }
    
    cout << "Rebuilt order index: " << orderIDToOffsetMap.size() << " orders.\n";
    saveIndex();
}