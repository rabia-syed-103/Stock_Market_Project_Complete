#include "Trade.h"

Trade:: Trade(int tid, Order* buyOrder, Order* sellOrder, int qty, double prc)
    : tradeID(tid),
        buyOrderID(buyOrder->orderID),
        sellOrderID(sellOrder->orderID),
        buyUserID(buyOrder->userID),
        sellUserID(sellOrder->userID),
        symbol(buyOrder->symbol),
        price(prc),
        quantity(qty)
{
    timestamp = std::time(nullptr);
}

// Displayable format
string Trade:: toString() const {
    std::ostringstream oss;
    oss << "TradeID: " << tradeID
        << " | Symbol: " << symbol
        << " | Qty: " << quantity
        << " | Price: $" << std::fixed << std::setprecision(2) << price
        << " | BuyOrder: " << buyOrderID << " (" << buyUserID << ")"
        << " | SellOrder: " << sellOrderID << " (" << sellUserID << ")"
        << " | Timestamp: " << timestamp;
    return oss.str();
}