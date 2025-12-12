#include "User.h"
#include <iostream>


User:: User()
{}
User::User(const std::string& uid, double cash)
    : userID(uid), cashBalance(cash) {
    // Vectors start empty, will grow dynamically
}

// Cash management
bool User::deductCash(double amount) {
    if (cashBalance < amount) {
        cout << " - INSUFFICIENT FUNDS\n";
        return false;
    }
    cashBalance -= amount;
    cout << " - SUCCESS, new balance=$" << cashBalance << "\n";
    return true;
}

void User::addCash(double amount) {
    cashBalance += amount;
}

double User::getCashBalance() const {
    return cashBalance;
}

// Stock management
void User::addStock(const string& symbol, int qty) {
    int idx = findSymbol(symbol);
    if (idx == -1) {
        // New stock - add to vectors
        symbols.push_back(symbol);
        quantities.push_back(qty);
    } else {
        // Existing stock - increase quantity
        quantities[idx] += qty;
    }
}

bool User::removeStock(const string& symbol, int qty) {
    int idx = findSymbol(symbol);
    if (idx == -1) return false;
    if (quantities[idx] < qty) return false;
    
    quantities[idx] -= qty;
    return true;
}

int User::getStockQuantity(const string& symbol) const {
    int idx = findSymbol(symbol);
    if (idx == -1) return 0;
    return quantities[idx];
}

// Order management
void User::addActiveOrder(int orderID) {
    activeOrders.push_back(orderID);
}

void User::removeActiveOrder(int orderID) {
    for (size_t i = 0; i < activeOrders.size(); i++) {
        if (activeOrders[i] == orderID) {
            activeOrders[i] = activeOrders.back();
            activeOrders.pop_back();
            return;
        }
    }
}

// Getters
string User::getUserID() const {
    return userID;
}

// Helper methods
int User::findSymbol(const string& symbol) const {
    for (size_t i = 0; i < symbols.size(); i++) {
        if (symbols[i] == symbol) return i;
    }
    return -1;
}

// Display
string User::toString() const {
    std::ostringstream oss;
    oss << "User: " << userID << ", Cash: $" << cashBalance;
    
    oss << ", Holdings: ";
    for (size_t i = 0; i < symbols.size(); i++) {
        oss << "[" << symbols[i] << ":" << quantities[i] << "] ";
    }
    
    oss << ", Active Orders: ";
    for (size_t i = 0; i < activeOrders.size(); i++) {
        oss << activeOrders[i] << " ";
    }
    
    return oss.str();
}

vector<int> User:: getActiveOrderIDs() const
{
    return activeOrders;
    
}

vector<StockHolding> User:: getAllHoldings() const {
    vector<StockHolding> holdings;
    for (size_t i = 0; i < symbols.size(); i++) {
        if (quantities[i] > 0) {
            StockHolding h;
            h.symbol = symbols[i];
            h.quantity = quantities[i];
            holdings.push_back(h);
        }
    }
    return holdings;
}