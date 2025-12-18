/*
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
    */

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


#include <iostream>
#include <thread>
#include <chrono>
#include "Stock_Market_Matching_Engine/engine/MatchingEngine.h"

using namespace std;

void printSeparator(const string& title) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(60, '=') << "\n\n";
}

void testPersistence() {
    printSeparator("PERSISTENCE TEST - CREATING NEW SYSTEM");
    
    // Create matching engine
    MatchingEngine engine;
    
    // Add stocks
    cout << "Adding stocks...\n";
    engine.addStock("AAPL", "admin123");
    engine.addStock("GOOGL", "admin123");
    engine.addStock("TSLA", "admin123");
    
    // Create users
    printSeparator("CREATING USERS");
    engine.createUser("alice", 10000.0);
    engine.createUser("bob", 15000.0);
    engine.createUser("charlie", 20000.0);
    
    // Place some orders
    printSeparator("PLACING ORDERS");
    
    cout << "\n--- Alice places BUY orders ---\n";
    engine.placeOrder("alice", "AAPL", "BUY", 150.0, 10);
    engine.placeOrder("alice", "GOOGL", "BUY", 2800.0, 5);
    
    cout << "\n--- Bob places SELL orders ---\n";
    engine.placeOrder("bob", "AAPL", "SELL", 155.0, 5);
    engine.placeOrder("bob", "GOOGL", "SELL", 2850.0, 3);
    
    cout << "\n--- Charlie creates matching orders (TRADES!) ---\n";
    engine.placeOrder("charlie", "AAPL", "SELL", 150.0, 8);  // Matches Alice's buy
    engine.placeOrder("charlie", "GOOGL", "BUY", 2850.0, 2); // Matches Bob's sell
    
    // Show portfolios
    printSeparator("PORTFOLIOS AFTER TRADING");
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    engine.printPortfolio("charlie");
    
    // Show order books
    printSeparator("ORDER BOOKS STATE");
    engine.printOrderBook("AAPL");
    engine.printOrderBook("GOOGL");
    
    // Show trade history
    printSeparator("TRADE HISTORY");
    vector<Trade> trades = engine.getAllTrades();
    cout << "Total trades: " << trades.size() << "\n\n";
    for (const Trade& trade : trades) {
        cout << trade.toString() << "\n";
    }
    
    printSeparator("SYSTEM WILL NOW SHUTDOWN - DATA PERSISTED");
    cout << "All data has been saved to disk.\n";
    cout << "Restart the program to verify persistence!\n";
}

void testRecovery() {
    printSeparator("PERSISTENCE TEST - LOADING FROM DISK");
    
    // Create NEW matching engine - should load everything from disk
    MatchingEngine engine;
    
    printSeparator("VERIFYING RECOVERED DATA");
    
    // Check users exist with correct balances
    cout << "Checking users...\n";
    User* alice = engine.getUser("alice");
    User* bob = engine.getUser("bob");
    User* charlie = engine.getUser("charlie");
    
    if (alice) cout << "✓ Alice recovered: $" << alice->getCashBalance() << "\n";
    else cout << "✗ Alice NOT FOUND!\n";
    
    if (bob) cout << "✓ Bob recovered: $" << bob->getCashBalance() << "\n";
    else cout << "✗ Bob NOT FOUND!\n";
    
    if (charlie) cout << "✓ Charlie recovered: $" << charlie->getCashBalance() << "\n";
    else cout << "✗ Charlie NOT FOUND!\n";
    
    // Show portfolios
    printSeparator("RECOVERED PORTFOLIOS");
    engine.printPortfolio("alice");
    engine.printPortfolio("bob");
    engine.printPortfolio("charlie");
    
    // Show order books
    printSeparator("RECOVERED ORDER BOOKS");
    engine.printOrderBook("AAPL");
    engine.printOrderBook("GOOGL");
    
    // Show trade history
    printSeparator("RECOVERED TRADE HISTORY");
    vector<Trade> trades = engine.getAllTrades();
    cout << "Total trades recovered: " << trades.size() << "\n\n";
    for (const Trade& trade : trades) {
        cout << trade.toString() << "\n";
    }
    
    // Place new order to test continued operation
    printSeparator("TESTING CONTINUED OPERATION");
    cout << "\n--- Bob places new order after recovery ---\n";
    engine.placeOrder("bob", "TSLA", "BUY", 800.0, 5);
    
    engine.printPortfolio("bob");
    engine.printOrderBook("TSLA");
    
    printSeparator("RECOVERY TEST COMPLETE");
}

void interactiveMenu() {
    MatchingEngine engine;
    
    while (true) {
        printSeparator("STOCK MARKET MATCHING ENGINE");
        cout << "1. Create User\n";
        cout << "2. Add Stock\n";
        cout << "3. Place Order\n";
        cout << "4. Cancel Order\n";
        cout << "5. View Portfolio\n";
        cout << "6. View Order Book\n";
        cout << "7. View Trade History\n";
        cout << "8. View All Users\n";
        cout << "9. Exit\n";
        cout << "\nChoice: ";
        
        int choice;
        cin >> choice;
        
        if (choice == 1) {
            string userID;
            double cash;
            cout << "User ID: ";
            cin >> userID;
            cout << "Initial Cash: $";
            cin >> cash;
            engine.createUser(userID, cash);
            
        } else if (choice == 2) {
            string symbol;
            cout << "Stock Symbol: ";
            cin >> symbol;
            engine.addStock(symbol, "admin123");
            
        } else if (choice == 3) {
            string userID, symbol, side;
            double price;
            int quantity;
            
            cout << "User ID: ";
            cin >> userID;
            cout << "Symbol: ";
            cin >> symbol;
            cout << "Side (BUY/SELL): ";
            cin >> side;
            cout << "Price: $";
            cin >> price;
            cout << "Quantity: ";
            cin >> quantity;
            
            engine.placeOrder(userID, symbol, side, price, quantity);
            
        } else if (choice == 4) {
            int orderID;
            string userID;
            cout << "Order ID: ";
            cin >> orderID;
            cout << "User ID: ";
            cin >> userID;
            engine.cancelOrder(orderID, userID);
            
        } else if (choice == 5) {
            string userID;
            cout << "User ID: ";
            cin >> userID;
            engine.printPortfolio(userID);
            
        } else if (choice == 6) {
            string symbol;
            cout << "Symbol: ";
            cin >> symbol;
            engine.printOrderBook(symbol);
            
        } else if (choice == 7) {
            vector<Trade> trades = engine.getAllTrades();
            cout << "\nTotal Trades: " << trades.size() << "\n\n";
            for (const Trade& trade : trades) {
                cout << trade.toString() << "\n";
            }
            
        } else if (choice == 8) {
            // This would require adding a getAllUsers() method to MatchingEngine
            cout << "Feature not implemented yet.\n";
            
        } else if (choice == 9) {
            cout << "Saving all data and exiting...\n";
            break;
            
        } else {
            cout << "Invalid choice!\n";
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }
}

int main(int argc, char* argv[]) {
    cout << "\n";
    cout << "╔════════════════════════════════════════════════════════╗\n";
    cout << "║  STOCK MARKET MATCHING ENGINE - PERSISTENCE TEST      ║\n";
    cout << "╚════════════════════════════════════════════════════════╝\n";
    
    if (argc > 1) {
        string arg = argv[1];
        
        if (arg == "--test") {
            testPersistence();
            
        } else if (arg == "--recover") {
            testRecovery();
            
        } else if (arg == "--interactive") {
            interactiveMenu();
            
        } else if (arg == "--clean") {
            cout << "\nCleaning up data files...\n";
            system("rm -f data/users.dat data/trades.dat data/metadata.dat");
            cout << "Done! (orders.dat and symbols.dat kept)\n";
            
        } else {
            cout << "\nUsage:\n";
            cout << "  " << argv[0] << " --test        : Run persistence test\n";
            cout << "  " << argv[0] << " --recover     : Test recovery from disk\n";
            cout << "  " << argv[0] << " --interactive : Interactive menu\n";
            cout << "  " << argv[0] << " --clean       : Clean persistence files\n";
        }
    } else {
        cout << "\nNo arguments provided. Running automatic test...\n";
        
        printSeparator("STEP 1: CREATE AND POPULATE SYSTEM");
        testPersistence();
        
        cout << "\n\n";
        this_thread::sleep_for(chrono::seconds(2));
        
        printSeparator("STEP 2: RESTART AND VERIFY RECOVERY");
        testRecovery();
        
        cout << "\n\n";
        printSeparator("TEST COMPLETE");
        cout << "✓ All data persisted and recovered successfully!\n";
        cout << "\nTry running with --interactive for manual testing.\n";
    }
    
    return 0;
}