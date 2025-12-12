#include "Order.h"

Order::Order(int id, const string& uID, const string& sym, const string& sd,
             double prc, int qty) 
    : orderID(id), userID(uID), symbol(sym), side(sd),
      price(prc), quantity(qty), remainingQty(qty), status("ACTIVE")
{
    timestamp = std::time(nullptr);
}

// Check if order is completely filled
bool Order::isFilled() const {
    return remainingQty <= 0;
}

// Fill some quantity
void Order::fill(int qty) {
    if (qty > remainingQty) qty = remainingQty;
    remainingQty -= qty;
    if (remainingQty == 0) 
        status = "FILLED";
    else 
        status = "PARTIAL_FILL";
}

// Cancel order
void Order::cancel() {
    if (!isFilled()) {
        status = "CANCELLED";
        remainingQty = 0;
    }
}

string Order::toString() const {
    std::ostringstream oss;
    oss << "OrderID: " << orderID
        << ", User: " << userID
        << ", Symbol: " << symbol
        << ", Side: " << side
        << ", Price: $" << std::fixed << std::setprecision(2) << price
        << ", Qty: " << quantity
        << ", Remaining: " << remainingQty
        << ", Status: " << status
        << ", Timestamp: " << timestamp;
    return oss.str();
}

bool Order::getSide() const {
    return this->side == "BUY";
}

int Order::getRemainingQuantity() const {
    return this->remainingQty;
}

double Order::getPrice() const {
    return this->price;
}

string Order::getSymbol() const {
    return this->symbol;
}

int Order::getOrderID() const {
    return this->orderID;
}

void Order::reduceRemainingQty(int qty) {
    remainingQty -= qty;
    if (remainingQty <= 0) {
        remainingQty = 0;
        status = "FILLED";
    } else {
        status = "PARTIAL_FILL";
    }
}

