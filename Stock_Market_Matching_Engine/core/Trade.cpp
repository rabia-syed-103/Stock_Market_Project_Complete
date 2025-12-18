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


TradeRecord Trade::toRecord() const {
    TradeRecord rec;
    memset(&rec, 0, sizeof(TradeRecord));
    
    rec.tradeID = tradeID;
    rec.buyOrderID = buyOrderID;
    rec.sellOrderID = sellOrderID;
    
    strncpy(rec.buyUserID, buyUserID.c_str(), 63);
    strncpy(rec.sellUserID, sellUserID.c_str(), 63);
    strncpy(rec.symbol, symbol.c_str(), 31);
    
    rec.price = price;
    rec.quantity = quantity;
    rec.timestamp = timestamp;
    
    return rec;
}

Trade Trade::fromRecord(const TradeRecord& rec) {
    Trade trade;
    
    trade.tradeID = rec.tradeID;
    trade.buyOrderID = rec.buyOrderID;
    trade.sellOrderID = rec.sellOrderID;
    trade.buyUserID = string(rec.buyUserID);
    trade.sellUserID = string(rec.sellUserID);
    trade.symbol = string(rec.symbol);
    trade.price = rec.price;
    trade.quantity = rec.quantity;
    trade.timestamp = rec.timestamp;
    
    return trade;
}