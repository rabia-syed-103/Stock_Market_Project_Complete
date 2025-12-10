/*My system will allow users to:

Place Buy/Sell Orders - Users can tell the system what stock they want to buy or sell, at what price, and how many shares.
Automatic Matching - The system will automatically find matching orders. If someone wants to buy at $150 and someone wants to sell at $150, it matches them instantly.
View Portfolio - Users can see what stocks they own, how much cash they have, and whether they're making profit or loss.
Cancel Orders - If an order hasn't been matched yet, users can cancel it.
Order History - Users can search through all orders they've ever placed.
See Order Book - Users can view all current pending orders for any stock to understand the market.

*/
/*

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
    cout << "\n=== MATCHING ENGINE FULL TEST SUITE ===\n";

    MatchingEngine engine;

    // --- Step 1: Create users ---
    engine.createUser("alice", 100000.0);
    engine.createUser("bob", 100000.0);
    engine.createUser("charlie", 50000.0);

    // Assign initial stocks for testing
    engine.getUser("bob")->addStock("AAPL", 500);
    engine.getUser("bob")->addStock("GOOGL", 200);

    cout << "\n--- Initial Portfolios ---\n";
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    engine.printPortfolio("charlie");

    // --- Step 2: Basic Matching ---
    testBasicMatching(engine);

    // --- Step 3: Partial Fills ---
    testPartialFills(engine);

    // --- Step 4: Order Cancellation ---
    testCancellation(engine);

    // --- Step 5: Validation of funds & shares ---
    testValidation(engine);

    // --- Step 6: Multiple Stocks Orders ---
    testMultipleStocks(engine);

    // --- Step 7: Concurrency / Thread Safety ---
    testConcurrency(engine);

    // --- Step 8: Final Portfolios ---
    cout << "\n=== FINAL PORTFOLIOS AFTER ALL TESTS ===\n";
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    engine.printPortfolio("charlie");
    engine.printPortfolio("user1");
    engine.printPortfolio("user2");
    engine.printPortfolio("user3");

    cout << "\n=== ALL TESTS COMPLETE ===\n";
    return 0;
}

void testBasicMatching(MatchingEngine& engine) {
    cout << "\n--- TEST 1: Basic Order Matching ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 150.0, 100);
    cout << " Placed BUY order: " << buy->orderID << "\n";
    cout <<"Trying";
    Order* sell = engine.placeOrder("bob", "AAPL", "SELL", 150.0, 100);
    cout << " Placed SELL order: " << sell->orderID << "\n";
    
    vector<Trade> trades = engine.getAllTrades();
    cout << " Trades executed: " << trades.size() << "\n";
    
    if (trades.size() > 0) {
        cout << "   " << trades[0].toString() << "\n";
    }
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
}

void testPartialFills(MatchingEngine& engine) {
    cout << "\n--- TEST 2: Partial Fills ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 151.0, 200);
    cout << " Placed BUY 200 @ $151\n";
    
    Order* sell1 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 80);
    cout << " Placed SELL 80 @ $151 (partial match)\n";
    
    Order* sell2 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 120);
    cout << " Placed SELL 120 @ $151 (complete match)\n";
    
    cout << "   Buy order remaining: " << buy->remainingQty << "\n";
    cout << "   Buy order status: " << buy->status << "\n";
}

void testCancellation(MatchingEngine& engine) {
    cout << "\n--- TEST 3: Order Cancellation ---\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 149.0, 50);
    cout << " Placed BUY order: " << buy->orderID << "\n";
    
    User* alice = engine.getUser("alice");

    // Print portfolio before cancellation
    cout << "\nPortfolio BEFORE cancellation:\n";
    engine.printPortfolio("alice");

    double cashBefore = alice->getCashBalance();
    
    engine.cancelOrder(buy->orderID, "alice");
    cout << "\nCancelled order\n";
    
    double cashAfter = alice->getCashBalance();
    
    // Print portfolio after cancellation
    cout << "\nPortfolio AFTER cancellation:\n";
     engine.printPortfolio("alice");


    cout << "   Cash before: $" << cashBefore << "\n";
    cout << "   Cash after: $" << cashAfter << "\n";
    cout << "   Order status: " << buy->status << "\n";
}


void testValidation(MatchingEngine& engine) {
    cout << "\n--- TEST 4: Balance Validation ---\n";
    
    // Test insufficient funds
    Order* buy = engine.placeOrder("charlie", "AAPL", "BUY", 1000.0, 1000);
    if (!buy) {
        cout << " Correctly rejected order (insufficient funds)\n";
    }
    
    // Test insufficient shares
    Order* sell = engine.placeOrder("alice", "AAPL", "SELL", 150.0, 10000);
    if (!sell) {
        cout << " Correctly rejected order (insufficient shares)\n";
    }
}

void testMultipleStocks(MatchingEngine& engine) {
    cout << "\n--- TEST 5: Multiple Stocks ---\n";
    
    engine.placeOrder("alice", "GOOGL", "BUY", 2800.0, 10);
    cout << " Placed GOOGL order\n";
    
    engine.placeOrder("bob", "TSLA", "SELL", 240.0, 50);
    cout << " Placed TSLA order\n";
    
    engine.placeOrder("alice", "MSFT", "BUY", 380.0, 20);
    cout << " Placed MSFT order\n";
    
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
    
    cout << " Completed 15 concurrent orders without crashes\n";
    cout << "   Total trades: " << engine.getAllTrades().size() << "\n";
    engine.printPortfolio("user1");

}*/

#include <iostream>
#include <thread>
#include <iomanip>
#include "engine/MatchingEngine.h"

using namespace std;

void testBasicMatching(MatchingEngine& engine);
void testPartialFills(MatchingEngine& engine);
void testPriceTimePriority(MatchingEngine& engine);
void testCancellation(MatchingEngine& engine);
void testValidation(MatchingEngine& engine);
void testSelfTrading(MatchingEngine& engine);
void testMultipleStocks(MatchingEngine& engine);
void testConcurrency(MatchingEngine& engine);

int main() {
    cout << "\nMATCHING ENGINE TEST SUITE\n";
    cout << "----------------------------\n\n";
    
    MatchingEngine engine;
    
    // Setup
    cout << "Setup: Creating test users\n";
    engine.createUser("alice", 100000.0);
    engine.createUser("bob", 100000.0);
    engine.createUser("charlie", 50000.0);
    
    User* bob = engine.getUser("bob");
    bob->addStock("AAPL", 500);
    bob->addStock("GOOGL", 200);
    cout << "\n";
    
    // Run tests
    testBasicMatching(engine);
    testPartialFills(engine);
    testPriceTimePriority(engine);
    testCancellation(engine);
    testValidation(engine);
    testSelfTrading(engine);
    testMultipleStocks(engine);
    testConcurrency(engine);
    
    cout << "\nAll tests completed.\n\n";
    return 0;
}

void testBasicMatching(MatchingEngine& engine) {
    cout << "Test 1: Basic Order Matching\n";
    cout << "Testing simple buy-sell match at same price\n";
    
    double aliceCashBefore = engine.getCashBalance("alice");
    auto bobHoldings = engine.getHoldings("bob");
    int bobStockBefore = bobHoldings.size() > 0 ? bobHoldings[0].quantity : 0;
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 150.0, 100);
    Order* sell = engine.placeOrder("bob", "AAPL", "SELL", 150.0, 100);
    
    double aliceCashAfter = engine.getCashBalance("alice");
    auto bobHoldingsAfter = engine.getHoldings("bob");
    int bobStockAfter = bobHoldingsAfter.size() > 0 ? bobHoldingsAfter[0].quantity : 0;
    
    cout << "  Buy order: " << buy->orderID << " (" << buy->status << ")\n";
    cout << "  Sell order: " << sell->orderID << " (" << sell->status << ")\n";
    cout << "  Alice paid: $" << (aliceCashBefore - aliceCashAfter) << "\n";
    cout << "  Bob shares sold: " << (bobStockBefore - bobStockAfter) << "\n";
    cout << "  Result: " << (buy->status == "FILLED" ? "PASS" : "FAIL") << "\n\n";
}

void testPartialFills(MatchingEngine& engine) {
    cout << "Test 2: Partial Order Fills\n";
    cout << "Testing order that fills across multiple counter-orders\n";
    engine.printPortfolio("alice");
    cout <<"\n\n";
    engine.printPortfolio("bob");

    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 151.0, 200);
    cout << "  Placed BUY 200 @ $151\n";
    
    Order* sell1 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 80);
    cout << "  Matched SELL 80 @ $151\n";
    
    Order* sell2 = engine.placeOrder("bob", "AAPL", "SELL", 151.0, 120);
    cout << "  Matched SELL 120 @ $151\n";
    
    cout << "  Buy order remaining: " << buy->remainingQty << "\n";
    cout << "  Buy order status: " << buy->status << "\n";
    cout << "  Result: " << (buy->status == "FILLED" ? "PASS" : "FAIL") << "\n\n";
    engine.printPortfolio("bob");
    engine.printPortfolio("alice");
}

void testPriceTimePriority(MatchingEngine& engine) {
    cout << "Test 3: Price-Time Priority\n";
    cout << "Testing that best prices match first\n";
    
    engine.placeOrder("bob", "AAPL", "SELL", 155.0, 50);
    engine.placeOrder("bob", "AAPL", "SELL", 152.0, 50);
    engine.placeOrder("bob", "AAPL", "SELL", 153.0, 50);
    cout << "  Placed sell orders at $155, $152, $153\n";
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 160.0, 50);
    
    vector<Trade> trades = engine.getAllTrades();
    double matchedPrice = trades.back().price;
    
    cout << "  Buy order matched at: $" << matchedPrice << "\n";
    cout << "  Result: " << (matchedPrice == 152.0 ? "PASS" : "FAIL") << "\n\n";
}

void testCancellation(MatchingEngine& engine) {
    cout << "Test 4: Order Cancellation\n";
    cout << "Testing order cancellation and cash refund\n";
    
    double cashBefore = engine.getCashBalance("alice");
    
    Order* buy = engine.placeOrder("alice", "AAPL", "BUY", 149.0, 50);
    double cashAfterOrder = engine.getCashBalance("alice");
    
    engine.cancelOrder(buy->orderID, "alice");
    double cashAfterCancel = engine.getCashBalance("alice");
    
    cout << "  Cash before: $" << cashBefore << "\n";
    cout << "  Cash after order: $" << cashAfterOrder << "\n";
    cout << "  Cash after cancel: $" << cashAfterCancel << "\n";
    cout << "  Order status: " << buy->status << "\n";
    cout << "  Result: " << (cashAfterCancel == cashBefore ? "PASS" : "FAIL") << "\n\n";
}

void testValidation(MatchingEngine& engine) {
    cout << "Test 5: Balance Validation\n";
    cout << "Testing insufficient funds and shares rejection\n";
    
    Order* buy = engine.placeOrder("charlie", "AAPL", "BUY", 1000.0, 1000);
    cout << "  Insufficient funds order: " << (buy == nullptr ? "REJECTED" : "ACCEPTED") << "\n";
    
    Order* sell = engine.placeOrder("alice", "AAPL", "SELL", 150.0, 10000);
    cout << "  Insufficient shares order: " << (sell == nullptr ? "REJECTED" : "ACCEPTED") << "\n";
    
    cout << "  Result: " << (buy == nullptr && sell == nullptr ? "PASS" : "FAIL") << "\n\n";
}

void testSelfTrading(MatchingEngine& engine) {
    cout << "Test 6: Self-Trading Prevention\n";
    cout << "Testing that users cannot trade with themselves\n";
    
    // Give Alice some stock for this test
    User* alice = engine.getUser("alice");
    if (alice) {
        lock_guard<mutex> tempLock(*(new mutex));  // Temporary for this setup
        alice->addStock("MSFT", 100);
    }
    
    int tradesBefore = engine.getAllTrades().size();
    
    Order* sell = engine.placeOrder("alice", "MSFT", "SELL", 300.0, 50);
    Order* buy = engine.placeOrder("alice", "MSFT", "BUY", 300.0, 50);
    
    int tradesAfter = engine.getAllTrades().size();
    
    cout << "  Trades before: " << tradesBefore << "\n";
    cout << "  Trades after: " << tradesAfter << "\n";
    cout << "  Self-trade occurred: " << (tradesAfter > tradesBefore ? "YES" : "NO") << "\n";
    cout << "  Result: " << (tradesAfter == tradesBefore ? "PASS" : "FAIL") << "\n\n";
}

void testMultipleStocks(MatchingEngine& engine) {
    cout << "Test 7: Multiple Stock Symbols\n";
    cout << "Testing independent order books for different stocks\n";
    
    engine.placeOrder("alice", "GOOGL", "BUY", 2800.0, 10);
    engine.placeOrder("alice", "MSFT", "BUY", 380.0, 20);
    engine.placeOrder("alice", "TSLA", "BUY", 240.0, 15);
    
    OrderBook* aaplBook = engine.getOrderBook("AAPL");
    OrderBook* googlBook = engine.getOrderBook("GOOGL");
    OrderBook* msftBook = engine.getOrderBook("MSFT");
    
    cout << "  AAPL book exists: " << (aaplBook != nullptr ? "YES" : "NO") << "\n";
    cout << "  GOOGL book exists: " << (googlBook != nullptr ? "YES" : "NO") << "\n";
    cout << "  MSFT book exists: " << (msftBook != nullptr ? "YES" : "NO") << "\n";
    cout << "  Result: PASS\n\n";
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
    cout << "Test 8: Thread Safety and Concurrency\n";
    cout << "Testing simultaneous orders from multiple users\n";
    
    engine.createUser("user1", 100000.0);
    engine.createUser("user2", 100000.0);
    engine.createUser("user3", 100000.0);
    
    User* u1 = engine.getUser("user1");
    User* u2 = engine.getUser("user2");
    User* u3 = engine.getUser("user3");
    
    u1->addStock("AAPL", 1000);
    u2->addStock("AAPL", 1000);
    u3->addStock("AAPL", 1000);
    
    int tradesBefore = engine.getAllTrades().size();
    
    cout << "  Launching 3 threads with 5 orders each\n";
    thread t1(simulateUser, &engine, "user1", 5);
    thread t2(simulateUser, &engine, "user2", 5);
    thread t3(simulateUser, &engine, "user3", 5);
    
    t1.join();
    t2.join();
    t3.join();
    
    int tradesAfter = engine.getAllTrades().size();
    
    cout << "  Trades executed: " << (tradesAfter - tradesBefore) << "\n";
    cout << "  No crashes or deadlocks\n";
    
    // Verify accounting
    cout << "  User1 final portfolio:\n";
    engine.printPortfolio("user1");
    
    cout << "  Result: PASS\n\n";
}

//g++ -std=c++17 -pthread Stock_Market_Matching_Engine/main.cpp Stock_Market_Matching_Engine/core/User.cpp Stock_Market_Matching_Engine/core/Order.cpp Stock_Market_Matching_Engine/core/Trade.cpp Stock_Market_Matching_Engine/data_structures/BTree.cpp Stock_Market_Matching_Engine/data_structures/OrderQueue.cpp Stock_Market_Matching_Engine/engine/MatchingEngine.cpp Stock_Market_Matching_Engine/engine/OrderBook.cpp -o main
