#include "Order.h"


Order::Order()
{
}

Order::Order(int id, const string &uID, const string &sym, const string &sd,
             double prc, int qty)
    : orderID(id), userID(uID), symbol(sym), side(sd),
      price(prc), quantity(qty), remainingQty(qty), status("ACTIVE")
{
    timestamp = std::time(nullptr);
}

OrderRecord Order::toRecord() const {
    OrderRecord rec{};
    rec.orderID = orderID;

    strncpy(rec.userID, userID.c_str(), sizeof(rec.userID) - 1);
    strncpy(rec.symbol, symbol.c_str(), sizeof(rec.symbol) - 1);

    rec.side = (side == "BUY") ? 'B' : 'S';
    rec.price = price;
    rec.quantity = quantity;
    rec.remainingQty = remainingQty;

    if (status == "ACTIVE") rec.status = 'A';
    else if (status == "FILLED") rec.status = 'F';
    else if (status == "PARTIAL_FILL") rec.status = 'P';
    else rec.status = 'C';

    rec.timestamp = static_cast<int64_t>(timestamp);
    return rec;
}

Order Order::fromRecord(const OrderRecord& rec) {
    Order o(
        rec.orderID,
        rec.userID,
        rec.symbol,
        (rec.side == 'B') ? "BUY" : "SELL",
        rec.price,
        rec.quantity
    );

    o.remainingQty = rec.remainingQty;
    o.timestamp = rec.timestamp;

    if (rec.status == 'A') o.status = "ACTIVE";
    else if (rec.status == 'F') o.status = "FILLED";
    else if (rec.status == 'P') o.status = "PARTIAL_FILL";
    else o.status = "CANCELLED";

    return o;
}


bool Order::isFilled() const {
    return remainingQty <= 0;
}

void Order::fill(int qty) {
    if (qty > remainingQty) qty = remainingQty;
    remainingQty -= qty;
    if (remainingQty == 0)
        status = "FILLED";
    else
        status = "PARTIAL_FILL";
}

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
        << ", Price: $" << fixed << setprecision(2) << price
        << ", Qty: " << quantity
        << ", Remaining: " << remainingQty
        << ", Status: " << status
        << ", Timestamp: " << timestamp;
    return oss.str();
}

bool Order::getSide() const {
    return side == "BUY";
}

int Order::getRemainingQuantity() const {
    return remainingQty;
}

double Order::getPrice() const {
    return price;
}

string Order::getSymbol() const {
    return symbol;
}

int Order::getOrderID() const {
    return orderID;
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
