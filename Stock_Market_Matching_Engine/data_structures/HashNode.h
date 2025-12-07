#ifndef HASHNODE_H
#define HASHNODE_H
#include "../core/Order.h"

class HashNode {
public:
    int key;
    Order* value;
    HashNode* next;

    HashNode(int k, Order* v) {
        key = k;
        value = v;
        next = nullptr;
    }
};

#endif
