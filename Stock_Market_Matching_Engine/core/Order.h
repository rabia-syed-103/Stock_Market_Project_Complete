#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdint>
#include <cstring>

using namespace std;


struct OrderRecord {
    int32_t orderID;
    char userID[32];
    char symbol[8];
    char side;        // 'B' or 'S'
    double price;
    int32_t quantity;
    int32_t remainingQty;
    char status;      // 'A','F','P','C'
    int64_t timestamp;
};


class Order {
public:
    int orderID;
    string userID;
    string symbol;
    string side;        // "BUY" or "SELL"
    double price;
    int quantity;
    int remainingQty;
    string status;      // "ACTIVE", "FILLED", "PARTIAL_FILL", "CANCELLED"
    time_t timestamp;

    // Constructor
    Order();
    Order(int id, const string& uID, const string& sym, const string& sd,
          double prc, int qty);
        Order(const Order& other) = default;
    Order& operator=(const Order& other) = default;
    Order(Order&&) = default;
    Order& operator=(Order&&) = default;
    ~Order() = default;

    // Persistence helpers
    OrderRecord toRecord() const;
    static Order fromRecord(const OrderRecord& rec);

    // Methods
    bool isFilled() const;
    void fill(int qty);
    void cancel();
    string toString() const;

    // Getters
    bool getSide() const;
    int getRemainingQuantity() const;
    double getPrice() const;
    string getSymbol() const;
    int getOrderID() const;
    void reduceRemainingQty(int qty);
};

#endif
