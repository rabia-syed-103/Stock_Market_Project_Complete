#ifndef USER_H
#define USER_H

#include <string>
#include <sstream>

#define MAX_STOCKS 50         // Max number of unique symbols user can hold
#define MAX_ACTIVE_ORDERS 200 // Max active orders per user
using namespace std;

class User {
public:
    std::string userID;
    double cashBalance;

    // Holdings implementation WITHOUT MAP
    std::string symbols[MAX_STOCKS];
    int quantities[MAX_STOCKS];
    int holdingsCount = 0;

    // Active orders WITHOUT vector
    int activeOrders[MAX_ACTIVE_ORDERS];
    int activeOrderCount = 0;

public:
    User(const std::string& uid, double cash);
    bool deductCash(double amount);

    void addCash(double amount);

    // Add or increase stock holding
    void addStock(const std::string& symbol, int qty);

    // Remove stock, return false if insufficient quantity
    bool removeStock(const std::string& symbol, int qty);

    // ---------------- Helper methods ----------------

    // Find stock index
    int findSymbol(const std::string& symbol) const;

    // Add active order ID
    void addActiveOrder(int orderID);
    // Remove active order ID
    void removeActiveOrder(int orderID);

    // For debugging
    string toString() const;
};

#endif // USER_H
