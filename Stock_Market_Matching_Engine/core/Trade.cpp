#include "Trade.h"
#include <stdexcept>
#include <iostream>
Trade::Trade()
{
}

Trade::Trade(int tid, const Order& o1, const Order& o2, int qty, double prc)
{
    std::cout << "Reached Here!";
    if (o1.symbol != o2.symbol)
        throw runtime_error("Cannot trade orders with different symbols");

    if (o1.side == "BUY") {
        buyOrderID  = o1.orderID;
        buyUserID   = o1.userID;
        sellOrderID = o2.orderID;
        sellUserID  = o2.userID;
    } else {
        buyOrderID  = o2.orderID;
        buyUserID   = o2.userID;
        sellOrderID = o1.orderID;
        sellUserID  = o1.userID;
    }

    tradeID   = tid;
    symbol    = o1.symbol;
    price     = prc;
    quantity  = qty;
    timestamp = std::time(nullptr);
}

// Displayable format
string Trade:: toString() const {
    std::ostringstream oss;
    oss << "TradeID: " << tradeID
        << " , Symbol: " << symbol
        << " , Qty: " << quantity
        << " , Price: $" << std::fixed << std::setprecision(2) << price
        << " , BuyOrder: " << buyOrderID << " (" << buyUserID << ")"
        << " , SellOrder: " << sellOrderID << " (" << sellUserID << ")"
        << " , Timestamp: " << timestamp;
    return oss.str();
}