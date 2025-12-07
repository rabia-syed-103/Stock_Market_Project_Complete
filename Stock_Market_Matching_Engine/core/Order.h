#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

class Order {
public:
    int orderID;            // Unique order identifier
    string userID;     // Who placed the order
    string symbol;     // Stock symbol (AAPL, GOOGL, etc.)
    string side;       // "BUY" or "SELL"
    double price;           // Price per share
    int quantity;           // Original quantity
    int remainingQty;       // Remaining quantity to be filled
    time_t timestamp;       // Time when order was placed
    string status;     // "ACTIVE", "FILLED", "PARTIAL_FILL", "CANCELLED"

public:
    // Constructor
    Order(int id, const string& uID, const string& sym, const string& sd,double prc, int qty);
    bool isFilled() const;
    void fill(int qty);
    void cancel();
    string toString() const;
    bool getSide();
    int getRemainingQuantity();
    int getPrice();

};

#endif // ORDER_H