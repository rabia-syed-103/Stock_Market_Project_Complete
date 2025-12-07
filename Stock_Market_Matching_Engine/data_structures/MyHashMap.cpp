#include "MyHashMap.h"

MyHashMap:: MyHashMap(int cap = 10) {
    capacity = cap;
    table = new HashNode*[capacity];

    for (int i = 0; i < capacity; i++)
        table[i] = nullptr;
}

void MyHashMap:: insert(int key, Order* value) {
    int index = hashFunction(key);
    HashNode* head = table[index];

    // 1. Check if already exists → update
    while (head != nullptr) {
        if (head->key == key) {
            head->value = value;
            return;
        }
        head = head->next;
    }

    // 2. Insert new node at head
    HashNode* newNode = new HashNode(key, value);
    newNode->next = table[index];
    table[index] = newNode;
}

Order* MyHashMap:: get(int key) {
    int index = hashFunction(key);
    HashNode* head = table[index];

    while (head != nullptr) {
        if (head->key == key)
            return head->value;
        head = head->next;
    }
    return nullptr;
}

void MyHashMap:: remove(int key) {
    int index = hashFunction(key);
    HashNode* head = table[index];
    HashNode* prev = nullptr;

    while (head != nullptr) {
        if (head->key == key) {
            if (prev == nullptr)
                table[index] = head->next;
            else
                prev->next = head->next;
            delete head;
            return;
        }
        prev = head;
        head = head->next;
    }
}

MyHashMap:: ~MyHashMap() {
    for (int i = 0; i < capacity; i++) {
        HashNode* head = table[i];
        while (head != nullptr) {
            HashNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
    delete[] table;
}