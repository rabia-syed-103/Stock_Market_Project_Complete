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


UserRecord User::toRecord() const {
    UserRecord rec;
    memset(&rec, 0, sizeof(UserRecord));
    
    strncpy(rec.userID, userID.c_str(), 63);
    rec.cashBalance = cashBalance;
    
    // Convert holdings
    rec.numHoldings = min((int)symbols.size(), 50);
    for (int i = 0; i < rec.numHoldings; i++) {
        strncpy(rec.holdings[i].symbol, symbols[i].c_str(), 31);
        rec.holdings[i].quantity = quantities[i];
    }
    
    // Convert active orders
    rec.numActiveOrders = min((int)activeOrders.size(), 100);
    for (int i = 0; i < rec.numActiveOrders; i++) {
        rec.activeOrderIDs[i] = activeOrders[i];
    }
    
    return rec;
}

User User::fromRecord(const UserRecord& rec) {
    User user(string(rec.userID), rec.cashBalance);
    
    // Restore holdings
    for (int i = 0; i < rec.numHoldings; i++) {
        user.symbols.push_back(string(rec.holdings[i].symbol));
        user.quantities.push_back(rec.holdings[i].quantity);
    }
    
    return user;
}