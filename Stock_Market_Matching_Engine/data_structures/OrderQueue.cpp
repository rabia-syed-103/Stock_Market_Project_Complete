#include "OrderQueue.h"
#include "../storage/OrderStorage.h"
#include <iostream>

using namespace std;

OrderQueue::OrderQueue() : front(nullptr), rear(nullptr), size(0) {}

OrderQueue::~OrderQueue() {
    while (front) {
        OrderNode* temp = front;
        front = front->next;
        delete temp;
    }
}

void OrderQueue::enqueue(DiskOffset orderOffset) {
    OrderNode* node = new OrderNode{orderOffset, nullptr};

    if (!rear) {
        front = rear = node;
    } else {
        rear->next = node;
        rear = node;
    }
    size++;
}

DiskOffset OrderQueue::dequeue() {
    if (!front) return 0;  // Empty queue

    OrderNode* temp = front;
    DiskOffset offset = temp->orderOffset;

    front = front->next;
    if (!front) rear = nullptr;

    delete temp;
    size--;
    return offset;
}

DiskOffset OrderQueue::peek() const {
    if (!front) return 0;
    return front->orderOffset;
}

int OrderQueue::getSize() const {
    return size;
}

DiskOffset OrderQueue::removeOrder(int orderID, OrderStorage& storage) {
    OrderNode* prev = nullptr;
    OrderNode* curr = front;

    while (curr) {
        Order order = storage.load(curr->orderOffset);
        if (order.getOrderID() == orderID) {
            DiskOffset removedOffset = curr->orderOffset;

            if (!prev) front = curr->next;
            else prev->next = curr->next;

            if (curr == rear) rear = prev;

            delete curr;
            size--;
            return removedOffset;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;  // Not found
}

void OrderQueue::printQueue(OrderStorage& storage) const {
    OrderNode* current = front;
    while (current) {
        Order o = storage.load(current->orderOffset);
        cout << o.toString() << endl;
        current = current->next;
    }
}

void OrderQueue::printDetailedQueue(OrderStorage& storage) const {
    OrderNode* current = front;
    while (current) {
        Order o = storage.load(current->orderOffset);
        cout << "OrderID: " << o.getOrderID()
             << " , User: " << o.userID
             << " , Qty: " << o.getRemainingQuantity()
             << " , Price: $" << o.getPrice()
             << " , Status: " << o.status << "\n";
        current = current->next;
    }
}
