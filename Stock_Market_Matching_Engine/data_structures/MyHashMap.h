#ifndef MYHASHMAP_H
#define MYHASHMAP_H
#include "HashNode.h"

class MyHashMap {
private:
    HashNode** table;   // array of node pointers
    int capacity;

    int hashFunction(int key) {
        return key % capacity;
    }

public:
    MyHashMap(int cap = 10);
    void insert(int key, Order* value);
    Order* get(int key);
    void remove(int key);
    ~MyHashMap();
};


#endif
