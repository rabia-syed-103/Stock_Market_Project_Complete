#include <iostream>
#include <thread>
#include "engine/MatchingEngine.h"
using namespace std;

void testBasicMatching(MatchingEngine& engine);
void testPartialFills(MatchingEngine& engine);
void testCancellation(MatchingEngine& engine);
void testValidation(MatchingEngine& engine);
void testMultipleStocks(MatchingEngine& engine);
void testConcurrency(MatchingEngine& engine);

int main() {
    cout << "\n=== MATCHING ENGINE TEST SUITE ===\n";
    
    MatchingEngine engine;
    
    // Create test users
    engine.createUser("alice", 100000.0);
    engine.createUser("bob", 100000.0);
    engine.createUser("charlie", 50000.0);
    
    // Give some users initial stock
    User* bob = engine.getUser("bob");
    bob->addStock("AAPL", 500);
    bob->addStock("GOOGL", 200);
    
    cout << "\n✅ Test users created\n";
    
    // Run all tests
    testBasicMatching(engine);
    testPartialFills(engine);
    testCancellation(engine);
    testValidation(engine);
    testMultipleStocks(engine);
    testConcurrency(engine);
    
    cout << "\n=== ALL TESTS COMPLETE ===\n";
    return 0;
}

void testBasicMatching(MatchingEngine& engine) {
    cout << "\n--- TEST 1: Basic Order Matching ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 150.0, 100);
    cout << "✅ Placed BUY order: " << buy->orderID << "\n";
    
    Order* sell = engine.placeOrder("bob", "AAPL", "SELL", 150.0, 100);
    cout << "✅ Placed SELL order: " << sell->orderID << "\n";
    
    vector<Trade> trades = engine.getAllTrades();
    cout << "✅ Trades executed: " << trades.size() << "\n";
    
    if (trades.size() > 0) {
        cout << "   " << trades[0].toString() << "\n";
    }
}

void testPartialFills(MatchingEngine& engine) {
    cout << "\n--- TEST 2: Partial Fills ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 151.0, 200);
    cout << "✅ Placed BUY 200 @ $151\n";
    
    Order* sell1 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 80);
    cout << "✅ Placed SELL 80 @ $151 (partial match)\n";
    
    Order* sell2 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 120);
    cout << "✅ Placed SELL 120 @ $151 (complete match)\n";
    
    cout << "   Buy order remaining: " << buy->remainingQty << "\n";
    cout << "   Buy order status: " << buy->status << "\n";
}

void testCancellation(MatchingEngine& engine) {
    cout << "\n--- TEST 3: Order Cancellation ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 149.0, 50);
    cout << "✅ Placed BUY order: " << buy->orderID << "\n";
    
    User* alice = engine.getUser("alice");
    double cashBefore = alice->cashBalance;
    
    engine.cancelOrder(buy->orderID, "alice");
    cout << "✅ Cancelled order\n";
    
    double cashAfter = alice->cashBalance;
    cout << "   Cash before: $" << cashBefore << "\n";
    cout << "   Cash after: $" << cashAfter << "\n";
    cout << "   Order status: " << buy->status << "\n";
}

void testValidation(MatchingEngine& engine) {
    cout << "\n--- TEST 4: Balance Validation ---\n";
    
    // Test insufficient funds
    Order* buy = engine.placeOrder("charlie", "AAPL", "BUY", 1000.0, 1000);
    if (!buy) {
        cout << "✅ Correctly rejected order (insufficient funds)\n";
    }
    
    // Test insufficient shares
    Order* sell = engine.placeOrder("alice", "AAPL", "SELL", 150.0, 10000);
    if (!sell) {
        cout << "✅ Correctly rejected order (insufficient shares)\n";
    }
}

void testMultipleStocks(MatchingEngine& engine) {
    cout << "\n--- TEST 5: Multiple Stocks ---\n";
    
    engine.placeOrder("alice", "GOOGL", "BUY", 2800.0, 10);
    cout << "✅ Placed GOOGL order\n";
    
    engine.placeOrder("bob", "TSLA", "SELL", 240.0, 50);
    cout << "✅ Placed TSLA order\n";
    
    engine.placeOrder("alice", "MSFT", "BUY", 380.0, 20);
    cout << "✅ Placed MSFT order\n";
    
    OrderBook* aaplBook = engine.getOrderBook("AAPL");
    OrderBook* googlBook = engine.getOrderBook("GOOGL");
    
    cout << "   AAPL best bid: " << (aaplBook->getBestBid() ? "exists" : "none") << "\n";
    cout << "   GOOGL best bid: " << (googlBook->getBestBid() ? "exists" : "none") << "\n";
}

void simulateUser(MatchingEngine* engine, string userID, int numOrders) {
    for (int i = 0; i < numOrders; i++) {
        double price = 150.0 + (rand() % 10);
        int qty = 10 + (rand() % 90);
        string side = (rand() % 2 == 0) ? "BUY" : "SELL";
        
        engine->placeOrder(userID, "AAPL", side, price, qty);
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void testConcurrency(MatchingEngine& engine) {
    cout << "\n--- TEST 6: Concurrent Orders (Thread Safety) ---\n";
    
    // Create more test users
    engine.createUser("user1", 100000.0);
    engine.createUser("user2", 100000.0);
    engine.createUser("user3", 100000.0);
    
    User* u1 = engine.getUser("user1");
    User* u2 = engine.getUser("user2");
    User* u3 = engine.getUser("user3");
    
    u1->addStock("AAPL", 1000);
    u2->addStock("AAPL", 1000);
    u3->addStock("AAPL", 1000);
    
    engine.printPortfolio("user1");
    // Launch concurrent threads
    thread t1(simulateUser, &engine, "user1", 5);
    thread t2(simulateUser, &engine, "user2", 5);
    thread t3(simulateUser, &engine, "user3", 5);
    
    t1.join();
    t2.join();
    t3.join();
    
    cout << "✅ Completed 15 concurrent orders without crashes\n";
    cout << "   Total trades: " << engine.getAllTrades().size() << "\n";
    engine.printPortfolio("user1");

    cout <<"Order Book of AAPL\n";

    engine.printOrderBook("AAPL");

}
/*
g++ -std=c++17 \
    Stock_Market_Matching_Engine/main.cpp \
    Stock_Market_Matching_Engine/core/Order.cpp \
    Stock_Market_Matching_Engine/core/Trade.cpp \
    Stock_Market_Matching_Engine/core/User.cpp \
    Stock_Market_Matching_Engine/engine/MatchingEngine.cpp \
    Stock_Market_Matching_Engine/engine/OrderBook.cpp \
    Stock_Market_Matching_Engine/data_structures/OrderQueue.cpp \
    Stock_Market_Matching_Engine/data_structures/BTree.cpp \
    -o matching_engine
*/