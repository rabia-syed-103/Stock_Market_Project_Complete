#include <iostream>
#include "../DiskManagement/BufferManager.h"
#include "../data_structures/BTree.h"
#include "../core/Order.h"

using namespace std;

int main() {
    cout << "🚀 Testing Disk-Based B-Tree" << endl;
    cout << "=============================" << endl;
    
    // Step 1: Create BufferManager
    BufferManager buffer("test_btree.db", 20);
    cout << "✅ BufferManager created" << endl;
    
    // Step 2: Create B-Tree
    BTree btree(&buffer, 3);
    cout << "✅ B-Tree created with root at page " 
         << btree.getRootPageID() << endl;
    
    // Step 3: Create and insert orders
    cout << "\n📝 Inserting orders..." << endl;
    
    for (int i = 0; i < 5; i++) {
        double price = 150.0 + i * 2;
        
        Order* order = new Order(
            1000 + i,
            "user_" + to_string(i),
            "AAPL",
            "BUY",
            price,
            100
        );
        
        cout << "   Inserting: Order " << order->orderID 
             << " @ $" << price << endl;
        
        btree.insert(price, order);
        delete order;
    }
    
    // Step 4: Flush to disk
    cout << "\n💾 Flushing to disk..." << endl;
    buffer.flushAll();
    
    // Step 5: Query operations
    cout << "\n🔍 Testing queries..." << endl;
    cout << "   Lowest price: $" << btree.getLowestKey() << endl;
    cout << "   Highest price: $" << btree.getHighestKey() << endl;
    
    Order* best = btree.getBest();
    if (best) {
        cout << "   Best order: #" << best->orderID 
             << " @ $" << best->price << endl;
        delete best;
    }
    
    // Step 6: Stats
    cout << "\n📊 Buffer Stats:" << endl;
    buffer.printStats();
    
    cout << "\n✅ Test completed successfully!" << endl;
    
    return 0;
}

/*
COMPILE:
g++ -o test test_nb.cpp \
    data_structures/BTree.cpp \
    DiskManagement/BufferManager.cpp \
    DiskManagement/PageManager.cpp \
    DiskManagement/LRUCache.cpp \
    core/Order.cpp \
    data_structures/OrderQueue.cpp

RUN:
./test
*/