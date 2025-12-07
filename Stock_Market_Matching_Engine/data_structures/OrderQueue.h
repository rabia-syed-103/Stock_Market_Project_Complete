// order_queue.h
#ifndef ORDER_QUEUE_H
#define ORDER_QUEUE_H

#include "../core/Order.h"


struct OrderNode {
    Order* order;
    OrderNode* next;
};

class OrderQueue {
private:
    OrderNode* front;
    OrderNode* rear;
    int size;

public:
    OrderQueue();
    ~OrderQueue();

    void enqueue(Order* order);
    Order* dequeue();
    Order* peek();   // front element without removing
    int getSize();
    void printQueue(); // For testing
    Order* removeOrder(int orderID);

};

#endif
