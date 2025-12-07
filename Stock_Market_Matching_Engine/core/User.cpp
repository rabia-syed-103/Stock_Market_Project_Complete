#include "User.h"

User :: User (const std::string& uid, double cash)
    : userID(uid), cashBalance(cash) {}

bool User:: deductCash(double amount) {
    if (cashBalance < amount) return false;
    cashBalance -= amount;
    return true;
}

void User:: addCash(double amount) {
    cashBalance += amount;
}

// Add or increase stock holding
void User:: addStock(const std::string& symbol, int qty) {
    int idx = findSymbol(symbol);
    if (idx == -1) {
        symbols[holdingsCount] = symbol;
        quantities[holdingsCount] = qty;
        holdingsCount++;
    } else {
        quantities[idx] += qty;
    }
}

// Remove stock, return false if insufficient quantity
bool User:: removeStock(const std::string& symbol, int qty) {
    int idx = findSymbol(symbol);
    if (idx == -1) return false;
    if (quantities[idx] < qty) return false;

    quantities[idx] -= qty;
    return true;
}

// ---------------- Helper methods ----------------

// Find stock index
int User:: findSymbol(const std::string& symbol) const {
    for (int i = 0; i < holdingsCount; i++) {
        if (symbols[i] == symbol) return i;
    }
    return -1;
}

// Add active order ID
void User:: addActiveOrder(int orderID) {
    activeOrders[activeOrderCount++] = orderID;
}

// Remove active order ID
void User:: removeActiveOrder(int orderID) {
    for (int i = 0; i < activeOrderCount; i++) {
        if (activeOrders[i] == orderID) {
            activeOrders[i] = activeOrders[activeOrderCount - 1];
            activeOrderCount--;
            return;
        }
    }
}

// For debugging
string User:: toString() const {
    std::ostringstream oss;
    oss << "User: " << userID << " | Cash: " << cashBalance;

    oss << " | Holdings: ";
    for (int i = 0; i < holdingsCount; i++) {
        oss << "[" << symbols[i] << ":" << quantities[i] << "] ";
    }

    oss << " | Active Orders: ";
    for (int i = 0; i < activeOrderCount; i++) {
        oss << activeOrders[i] << " ";
    }

    return oss.str();
}