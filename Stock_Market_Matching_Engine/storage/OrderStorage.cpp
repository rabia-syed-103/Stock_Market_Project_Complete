
#include "OrderStorage.h"
#include "../core/Order.h"
#include <fstream>


OrderStorage::OrderStorage() : storage("data/orders.dat") {
    const size_t recSize = sizeof(OrderRecord);
    std::ifstream f("data/orders.dat", std::ios::binary | std::ios::ate);
    if (!f) return;
    size_t fileSize = (size_t)f.tellg();
    f.close();

    // raw file offsets start at 0, but we store shifted offsets (raw + 1)
    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        OrderRecord rec;
        storage.read(rawOff, &rec, recSize);
        Order o = Order::fromRecord(rec);
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1; // shift so 0 remains sentinel
        ordersMap[storedOff] = o;
        orderIDToOffsetMap[o.orderID] = storedOff;
    }
}
OrderStorage::~OrderStorage()
{
}

// Persist a new order
DiskOffset OrderStorage::persist(const Order& order) {
    // write a POD record to disk, not the C++ Order object
    OrderRecord rec = order.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(OrderRecord));
    // rawOff is the actual file offset (may be 0 for first record) — shift by +1
    DiskOffset storedOff = rawOff + 1;
    ordersMap[storedOff] = order;
    orderIDToOffsetMap[order.orderID] = storedOff;
    return storedOff;
}

// Load order from offset
Order OrderStorage::load(DiskOffset offset) {
    if (offset == 0) return Order(); // invalid
    DiskOffset rawOff = offset - 1;
    OrderRecord rec;
    storage.read(rawOff, &rec, sizeof(OrderRecord));
    return Order::fromRecord(rec);
}

// Save order back to disk
void OrderStorage::save(const Order& order, DiskOffset offset) {
    if (offset == 0) return;
    DiskOffset rawOff = offset - 1;
    OrderRecord rec = order.toRecord();
    storage.write(rawOff, &rec, sizeof(OrderRecord));
    ordersMap[offset] = order;
}

// Load all orders for a symbol
vector<Order> OrderStorage::loadAllOrdersForSymbol(const string& symbol) {
    vector<Order> result;
    for (auto& [offset, order] : ordersMap) {
        if (order.symbol == symbol)
            result.push_back(order);
    }
    return result;
}

// Get offset for an orderID
DiskOffset OrderStorage::getOffsetForOrder(int orderID) {
    return orderIDToOffsetMap[orderID];
}

vector<Order> OrderStorage::loadAllOrders() {
    vector<Order> result;
    for (auto& [offset, order] : ordersMap) {
        result.push_back(order);
    }
    return result;
}