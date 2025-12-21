#include <iostream>
#include <cassert>
#include "engine/PersistentMatchingEngine.h"

using namespace std;

/* ================= PHASE 1 =================
   Create data, execute trades, leave open orders
   ========================================== */
void phase1_create_and_persist() {
    cout << "\n===== PHASE 1: CREATE & PERSIST =====\n";
    PersistentMatchingEngine engine;
    
    assert(engine.addStock("AAPL", "admin123") || engine.symbolExists("AAPL"));
    
    engine.createUser("alice", 10000);
    engine.createUser("bob", 10000);

    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    
    User* bob = engine.getUser("bob");
    assert(bob);
    bob->addStock("AAPL", 100);
    engine.printPortfolio("bob");
    
    Order* s1 = engine.placeOrder("bob", "AAPL", "SELL", 150, 50);
    assert(s1);
    
    Order* b1 = engine.placeOrder("alice", "AAPL", "BUY", 150, 30);
    assert(b1);
    
    // ✅ FIX: Reload orders from disk to get updated state
    //Order* s1_updated = engine.(s1->orderID);
    //Order* b1_updated = engine.getOrder(b1->orderID);
    
   // assert(b1_updated->getRemainingQuantity() == 0);
    //assert(s1_updated->getRemainingQuantity() == 20);
    
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");

    engine.cancelOrder(s1->orderID, "bob");
    
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    engine.printOrderBook("AAPL");
    
    auto trades = engine.getUserTrades("alice");
    assert(trades.size() == 1);
    
    cout << "\nPHASE 1 COMPLETE — EXIT PROGRAM\n";
}

/* ================= PHASE 2 =================
   Reload system, verify disk reconstruction
   ========================================== */
void phase2_recover_and_verify() {
    cout << "\n===== PHASE 2: RECOVER & VERIFY =====\n";

    PersistentMatchingEngine engine;

    // --- Users recovered ---
    User* alice = engine.getUser("alice");
    User* bob   = engine.getUser("bob");

    assert(alice);
    assert(bob);

    // --- Portfolio correctness ---
    cout << "\nRecovered portfolios:\n";
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");

    // Alice bought 30 @150 = 4500
    assert(alice->getStockQuantity("AAPL") == 30);
    assert(alice->getCashBalance() == 5500);

    // Bob sold 30 shares, remaining cancelled refunded
    assert(bob->getStockQuantity("AAPL") == 70);
    assert(bob->getCashBalance() == 14500);

    // --- OrderBook rebuilt ---
    cout << "\nRecovered order book:\n";
    engine.printOrderBook("AAPL");

    // --- Trades recovered ---
    auto trades = engine.getUserTrades("alice");
    assert(trades.size() == 1);
    cout << "\nRecovered trade:\n";
    cout << trades[0].toString() << "\n";

    // --- New order after recovery ---
    cout << "\nPlacing new order after restart...\n";
    engine.placeOrder("alice", "AAPL", "BUY", 140, 10);
    engine.printOrderBook("AAPL");

    cout << "\nPHASE 2 COMPLETE — TRUE PERSISTENCE VERIFIED\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "\nUsage:\n";
        cout << "  ./main phase1   # create & persist data\n";
        cout << "  ./main phase2   # recover & verify\n";
        return 0;
    }

    string mode = argv[1];

    if (mode == "phase1") {
        phase1_create_and_persist();
    } 
    else if (mode == "phase2") {
        phase2_recover_and_verify();
    } 
    else {
        cout << "Invalid mode\n";
    }

    return 0;
}

//g++ -std=c++17 -pthread \
    Stock_Market_Matching_Engine/main.cpp \
    Stock_Market_Matching_Engine/**/*.cpp \
    -o main
