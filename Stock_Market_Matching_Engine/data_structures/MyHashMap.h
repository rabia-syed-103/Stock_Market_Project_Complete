
#ifndef MYHASHMAP_H
#define MYHASHMAP_H

#include <cstring>
#include <string>
#include "HashNode.h"

template<typename K, typename V>
class MyHashMap {
private:
    HashNode<K, V>** table;
    int capacity;
    int size;
    
    // Hash function for int keys
    int hashFunction(int key) {
        return key % capacity;
    }
    
    // Hash function for string keys (djb2 algorithm)
    int hashFunction(const std::string& key) {
        unsigned long hash = 5381;
        for (char c : key) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash % capacity;
    }
    
    // Hash function for const char*
    int hashFunction(const char* key) {
        unsigned long hash = 5381;
        for (int i = 0; key[i] != '\0'; i++) {
            hash = ((hash << 5) + hash) + key[i];
        }
        return hash % capacity;
    }
    
    // Key comparison for different types
    bool keysEqual(const K& k1, const K& k2) {
        return k1 == k2;
    }
    
    // Specialization for const char*
    bool keysEqual(const char* k1, const char* k2) {
        return strcmp(k1, k2) == 0;
    }

public:
    MyHashMap(int cap = 100) : capacity(cap), size(0) {
        table = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(K key, V value) {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        
        // Check if key exists -> update
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                head->value = value;
                return;
            }
            head = head->next;
        }
        
        // Insert new node at head
        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        size++;
    }
    
    V get(K key) {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                return head->value;
            }
            head = head->next;
        }
        
        return nullptr;  // Not found
    }
    
    void remove(K key) {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        HashNode<K, V>* prev = nullptr;
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                if (prev == nullptr) {
                    table[index] = head->next;
                } else {
                    prev->next = head->next;
                }
                delete head;
                size--;
                return;
            }
            prev = head;
            head = head->next;
        }
    }
    
    bool contains(K key) {
        return get(key) != nullptr;
    }
    
    int getSize() const {
        return size;
    }
    
    ~MyHashMap() {
        for (int i = 0; i < capacity; i++) {
            HashNode<K, V>* head = table[i];
            while (head != nullptr) {
                HashNode<K, V>* temp = head;
                head = head->next;
                delete temp;
            }
        }
        delete[] table;
    }
};

#endif

