#include <iostream>
#include <cstring>
#include "BufferManager.h"

using namespace std;

// Helper functions to serialize/deserialize Orders

struct SimpleOrder {
    int orderID;
    char userID[50];
    char symbol[10];
    double price;
    int quantity;
};

// Serialize order to page buffer
void serializeOrder(char* pageBuffer, const SimpleOrder& order) {
    memset(pageBuffer, 0, PAGE_SIZE);
    
    int offset = 0;
    
    memcpy(pageBuffer + offset, &order.orderID, sizeof(int));
    offset += sizeof(int);
    
    memcpy(pageBuffer + offset, order.userID, 50);
    offset += 50;
    
    memcpy(pageBuffer + offset, order.symbol, 10);
    offset += 10;
    
    memcpy(pageBuffer + offset, &order.price, sizeof(double));
    offset += sizeof(double);
    
    memcpy(pageBuffer + offset, &order.quantity, sizeof(int));
}

// Deserialize order from page buffer
SimpleOrder deserializeOrder(const char* pageBuffer) {
    SimpleOrder order;
    
    int offset = 0;
    
    memcpy(&order.orderID, pageBuffer + offset, sizeof(int));
    offset += sizeof(int);
    
    memcpy(order.userID, pageBuffer + offset, 50);
    offset += 50;
    
    memcpy(order.symbol, pageBuffer + offset, 10);
    offset += 10;
    
    memcpy(&order.price, pageBuffer + offset, sizeof(double));
    offset += sizeof(double);
    
    memcpy(&order.quantity, pageBuffer + offset, sizeof(int));
    
    return order;
}

void demonstrateBlockBasedStorage() {
    cout << "\n🎯 DEMONSTRATION: Block-Based Order Storage" << endl;
    cout << "============================================\n" << endl;
    
    // Create BufferManager
    BufferManager bm("orders.db");
    
    // We'll maintain a simple hash table in memory: orderID → pageID
    // (In real implementation, this would be MyHashMap<int, int>)
    int orderIDToPageID[10];  // Simple array for demo
    
    cout << "📝 Step 1: Creating and storing 5 orders..." << endl;
    
    // Create 5 orders and store them in different pages
    for (int i = 0; i < 5; i++) {
        SimpleOrder order;
        order.orderID = 12345 + i;
        sprintf(order.userID, "user_%d", i);
        strcpy(order.symbol, "AAPL");
        order.price = 150.0 + i * 5;
        order.quantity = 100 + i * 10;
        
        // Allocate a new page for this order
        int pageID = bm.allocateNewPage();
        
        // Store mapping: orderID → pageID
        orderIDToPageID[i] = pageID;
        
        // Serialize order to buffer
        char* buffer = new char[PAGE_SIZE];
        serializeOrder(buffer, order);
        
        // Write to disk (via buffer manager)
        bm.updatePage(pageID, buffer);
        
        delete[] buffer;
        
        cout << "   Order " << order.orderID << " → Page " << pageID << endl;
    }
    
    cout << "\n💾 Step 2: Flushing to disk..." << endl;
    bm.flushAll();
    
    cout << "\n🔍 Step 3: Retrieving Order #12347..." << endl;
    
    // User wants to see order #12347
    int targetOrderID = 12347;
    int orderIndex = targetOrderID - 12345;  // Index in our simple array
    
    // Look up WHERE it's stored (O(1) hash lookup)
    int pageID = orderIDToPageID[orderIndex];
    cout << "   Hash lookup: Order " << targetOrderID << " is in page " << pageID << endl;
    
    // Load ONLY that page (4KB, not all orders!)
    char* pageData = bm.getPage(pageID);
    
    // Deserialize the order
    SimpleOrder retrieved = deserializeOrder(pageData);
    
    cout << "\n✅ Retrieved Order:" << endl;
    cout << "   OrderID: " << retrieved.orderID << endl;
    cout << "   UserID: " << retrieved.userID << endl;
    cout << "   Symbol: " << retrieved.symbol << endl;
    cout << "   Price: $" << retrieved.price << endl;
    cout << "   Quantity: " << retrieved.quantity << endl;
    
    cout << "\n📊 Step 4: BufferManager Stats" << endl;
    bm.printStats();
    
    cout << "\n🔄 Step 5: Updating Order #12345..." << endl;
    
    // Update order #12345
    int updateIndex = 0;
    pageID = orderIDToPageID[updateIndex];
    
    // Load the page
    pageData = bm.getPage(pageID);
    
    // Deserialize, modify, serialize back
    SimpleOrder toUpdate = deserializeOrder(pageData);
    toUpdate.quantity = 999;  // Change quantity
    
    char* updateBuffer = new char[PAGE_SIZE];
    serializeOrder(updateBuffer, toUpdate);
    
    bm.updatePage(pageID, updateBuffer);
    delete[] updateBuffer;
    
    cout << "   Updated Order " << toUpdate.orderID << " quantity to " << toUpdate.quantity << endl;
    
    cout << "\n💾 Step 6: Final flush" << endl;
    bm.flushAll();
    
    cout << "\n🎉 Demo complete!" << endl;
    cout << "\n💡 KEY TAKEAWAY:" << endl;
    cout << "   - We stored 5 orders" << endl;
    cout << "   - Only loaded 2-3 pages into memory (8-12 KB)" << endl;
    cout << "   - NOT all orders (would be MBs/GBs)" << endl;
    cout << "   - Hash table tells us WHERE to find data" << endl;
    cout << "   - BufferManager handles caching automatically" << endl;
}

int main() {
    demonstrateBlockBasedStorage();
    return 0;
}


/*
COMPILATION:
g++ -o demo test.cpp BufferManager.cpp PageManager.cpp LRUCache.cpp

RUN:
./demo
*/