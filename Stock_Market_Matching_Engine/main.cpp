
#include "engine/MatchingEngine.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    MatchingEngine engine;

    
    // 2️⃣ Add required stocks if missing
    std::vector<std::string> requiredStocks = {"AAPL", "GOOGL", "TSLA"};
    for (const std::string& sym : requiredStocks) {
        if (!engine.symbolExists(sym)) {  // symbolExists() checks symbolStorage/orderBooks
            cout << engine.addStock(sym, "admin123");
        }
    }

    // 3️⃣ Create users
    engine.createUser("admin123", 100000);
    engine.createUser("alice", 50000);
    engine.createUser("bob", 50000);

    User* bob = engine.getUser("bob");
    bob->addStock("AAPL", 500);
    bob->addStock("GOOGL", 200);
    int n1 =0,n2 =0;
    // 4️⃣ Users place orders
    engine.placeOrder("bob", "AAPL", "SELL", 150, 50);

    engine.placeOrder("alice", "AAPL", "BUY", 150, 50);

    engine.placeOrder("alice", "GOOGL", "BUY", 2500, 10);
  //  engine.placeOrder("alice", "GOOGL", "BUY", 2500, 10);


    // 5️⃣ Print portfolios
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");

    engine.placeOrder("alice", "AAPL", "BUY", 150, 10);
    engine.placeOrder("bob", "AAPL", "SELL", 150, 5);
    engine.printPortfolio("alice");

    engine.placeOrder("bob", "AAPL", "SELL", 150, 5);

    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    std::cout << "\n=== First main finished. Data saved to disk ===\n";
    return 0;
}
/*
#include "engine/MatchingEngine.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::cout << "=== Starting second main: rebuild from storage ===\n";

    // 1️⃣ Initialize engine
    MatchingEngine engine;



    // 3️⃣ Print all stocks and order books
    std::vector<std::string> stocks = {"AAPL", "GOOGL", "TSLA"};
    for (const std::string& sym : stocks) {
        std::cout << "\nOrder book for " << sym << ":\n";
        engine.printOrderBook(sym);
    }

    // 4️⃣ Print portfolios for all known users
    std::vector<std::string> users = {"admin123", "alice", "bob"};
    for (const std::string& u : users) {
        std::cout << "\nPortfolio for " << u << ":\n";
        engine.printPortfolio(u);
    }

    // 5️⃣ Optionally, show all trades executed
    std::vector<Trade> trades = engine.getAllTrades();
    std::cout << "\nAll trades executed:\n";
    for (const Trade& t : trades) {
        std::cout << t.toString() << "\n";
    }

    std::cout << "\n=== Second main finished: rebuilt from disk ===\n";
    return 0;
}



*/


//g++ -std=c++17 -pthread Stock_Market_Matching_Engine/main.cpp Stock_Market_Matching_Engine/core/User.cpp Stock_Market_Matching_Engine/core/Order.cpp Stock_Market_Matching_Engine/core/Trade.cpp Stock_Market_Matching_Engine/data_structures/BTree.cpp Stock_Market_Matching_Engine/data_structures/OrderQueue.cpp Stock_Market_Matching_Engine/engine/MatchingEngine.cpp Stock_Market_Matching_Engine/engine/OrderBook.cpp -o main

/*
g++ -std=c++17 -pthread \
    Stock_Market_Matching_Engine/main.cpp \
    Stock_Market_Matching_Engine/core/User.cpp \
    Stock_Market_Matching_Engine/core/Order.cpp \
    Stock_Market_Matching_Engine/core/Trade.cpp \
    Stock_Market_Matching_Engine/data_structures/BTree.cpp \
    Stock_Market_Matching_Engine/data_structures/OrderQueue.cpp \
    Stock_Market_Matching_Engine/engine/MatchingEngine.cpp \
    Stock_Market_Matching_Engine/engine/OrderBook.cpp \
    Stock_Market_Matching_Engine/storage/StorageManager.cpp \
    Stock_Market_Matching_Engine/storage/OrderStorage.cpp \
    -o main
*/

//g++ -std=c++17 -g -fsanitize=address -fno-omit-frame-pointer \
    Stock_Market_Matching_Engine/main.cpp \
    Stock_Market_Matching_Engine/**/*.cpp \
    -pthread -o main_asan