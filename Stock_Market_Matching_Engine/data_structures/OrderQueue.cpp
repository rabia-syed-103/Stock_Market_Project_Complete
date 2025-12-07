// order_queue.cpp
#include <iostream>
#include "OrderQueue.h"
using namespace std;

OrderQueue::OrderQueue() {
    front = nullptr;
    rear = nullptr;
    size = 0;
}

OrderQueue::~OrderQueue() {
    while (front != nullptr) {
        OrderNode* temp = front;
        front = front->next;
        delete temp;
    }
}

void OrderQueue::enqueue(Order* order) {
    OrderNode* node = new OrderNode;
    node->order = order;
    node->next = nullptr;

    if (rear == nullptr) { // empty queue
        front = rear = node;
    } else {
        rear->next = node;
        rear = node;
    }
    size++;
}

Order* OrderQueue::dequeue() {
    if (front == nullptr) return nullptr; // empty
    OrderNode* temp = front;
    Order* order = temp->order;
    front = front->next;
    if (front == nullptr) rear = nullptr;
    delete temp;
    size--;
    return order;
}

Order* OrderQueue::peek() {
    if (front == nullptr) return nullptr;
    return front->order;
}

int OrderQueue::getSize() {
    return size;
}

void OrderQueue::printQueue() {
    OrderNode* current = front;
    while (current != nullptr) {
        cout << current->order->toString() << endl;
        current = current->next;
    }
}

Order* OrderQueue::removeOrder(int orderID) {
    OrderNode* prev = nullptr;
    OrderNode* current = front;

    while (current) {
        if (current->order->orderID == orderID) {
            Order* ord = current->order;

            // Remove node safely
            if (prev == nullptr)
                front = current->next;
            else
                prev->next = current->next;

            if (current == rear)
                rear = prev;

            delete current;
            size--;
            return ord; // return pointer to order (caller can mark canceled)
        }
        prev = current;
        current = current->next;
    }

    return nullptr; // not found
}

void OrderQueue::printDetailedQueue() {
    OrderNode* current = front;
    while (current) {
        std::cout << "OrderID: " << current->order->orderID
                  << " | User: " << current->order->userID
                  << " | Qty: " << current->order->remainingQty
                  << " | Price: $" << current->order->price
                  << " | Status: " << current->order->status << "\n";
        current = current->next;
    }
}
