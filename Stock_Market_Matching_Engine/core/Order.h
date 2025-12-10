#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

class Order {
public:
    int orderID;
    string userID;
    string symbol;
    string side;        // "BUY" or "SELL"
    double price;       // DOUBLE for decimal prices
    int quantity;
    int remainingQty;
    string status;      // "ACTIVE", "FILLED", "PARTIAL_FILL", "CANCELLED"
    time_t timestamp;
    
    // Constructor
    Order(int id, const string& uID, const string& sym, const string& sd,
          double prc, int qty);
    
    // Methods
    bool isFilled() const;
    void fill(int qty);
    void cancel();
    string toString() const;
    
    // Getters
    bool getSide() const;           // Fixed: added const
    int getRemainingQuantity() const;  // Fixed: added const
    double getPrice() const;        // Fixed: added const
    string getSymbol() const;       // Added: useful getter
    int getOrderID() const;         // Added: useful getter
};

#endif