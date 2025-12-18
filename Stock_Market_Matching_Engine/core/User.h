#ifndef USER_H
#define USER_H

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "Order.h"

using namespace std;
struct StockHolding {
    string symbol;
    int quantity;
};
struct UserRecord {
    char userID[64];
    double cashBalance;
    
    // Holdings
    int numHoldings;
    struct {
        char symbol[32];
        int quantity;
    } holdings[50];
    
    // Active orders
    int numActiveOrders;
    int activeOrderIDs[100];
    
    // Padding
    char reserved[128];
};

class User {
private:
    string userID;
    double cashBalance;
    vector<string> symbols;
    vector<int> quantities;
    vector<int> activeOrders;
    
public:
    User();
    User(const string& uid, double cash);
    
    // Cash management
    bool deductCash(double amount);
    void addCash(double amount);
    double getCashBalance() const;
    
    // Stock management
    void addStock(const string& symbol, int qty);
    bool removeStock(const string& symbol, int qty);
    int getStockQuantity(const string& symbol) const;
    
    // Order management
    void addActiveOrder(int orderID);
    void removeActiveOrder(int orderID);
    
    // Getters
    string getUserID() const;
    vector<int> getActiveOrderIDs() const;
    vector<StockHolding> getAllHoldings() const ;
    
    // Helper
    int findSymbol(const string& symbol) const;
    
    // Display
    string toString() const;

    User clone() const { return *this; }

    static User fromRecord(const UserRecord& rec);
    UserRecord toRecord() const;


    
};

#endif