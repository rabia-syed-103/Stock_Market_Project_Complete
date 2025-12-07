#ifndef TRADE_H
#define TRADE_H

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "Order.h"
using namespace std;

class Trade {
public:
    int tradeID;            // Unique ID of trade
    int buyOrderID;         // Order ID of BUY order
    int sellOrderID;        // Order ID of SELL order
    string buyUserID;       // User who bought
    string sellUserID;      // User who sold
    string symbol;          // Stock symbol
    double price;           // Traded price per unit
    int quantity;           // Trade quantity
    time_t timestamp;       // Trade execution time

public:
    // Constructor
    Trade(int tid, Order* buyOrder, Order* sellOrder, int qty, double prc);

    // Displayable format
    string toString() const;
};

#endif // TRADE_H
