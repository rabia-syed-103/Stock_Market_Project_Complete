#ifndef ORDER_QUEUE_H
#define ORDER_QUEUE_H

#include "../storage/DiskTypes.h"  // DiskOffset type
#include <cstddef>

struct OrderNode {
    DiskOffset orderOffset;  // Disk offset of Order
    OrderNode* next;
};

class OrderStorage;  // Forward declaration

class OrderQueue {
private:
    OrderNode* front;
    OrderNode* rear;
    int size;

public:
    OrderQueue();
    ~OrderQueue();

    // Core queue operations
    void enqueue(DiskOffset orderOffset);      // Add order offset
    DiskOffset dequeue();                       // Remove and return offset
    DiskOffset peek() const;                    // Return front offset
    int getSize() const;

    // Remove specific order by ID
    DiskOffset removeOrder(int orderID, OrderStorage& storage);

    // Print the queue (loads orders from storage)
    void printQueue(OrderStorage& storage) const;
    void printDetailedQueue(OrderStorage& storage) const;
};

#endif
