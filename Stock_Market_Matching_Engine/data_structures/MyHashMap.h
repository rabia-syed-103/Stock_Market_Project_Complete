#ifndef MYHASHMAP_H
#define MYHASHMAP_H

#include <functional>
#include <stdexcept>
#include <cstring>
#include "HashNode.h"
#include <vector>
using namespace std;
template <typename K, typename V>
class MyHashMap {
private:
    HashNode<K, V>** table;
    int capacity;
    int size;
    
    // Generic hash function using std::hash
    size_t hashFunction(const K& key) const {
        return std::hash<K>{}(key) % capacity;
    }
    
    // Key comparison - works for any type with == operator
    bool keysEqual(const K& k1, const K& k2) const {
        return k1 == k2;
    }

public:
    MyHashMap(int cap = 100) : capacity(cap), size(0) {
        table = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(const K& key, const V& value) {
        int index = hashFunction(key);
        
        // Check if key already exists
        HashNode<K, V>* current = table[index];
        while (current != nullptr) {
            if (keysEqual(current->key, key)) {
                current->value = value; // Update existing value
                return;
            }
            current = current->next;
        }
        
        // Insert new node at the beginning of the chain
        HashNode<K, V>* node = new HashNode<K, V>(key, value);
        node->next = table[index];
        table[index] = node;
        size++;
    }
    
    V get(const K& key) const {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                return head->value;
            }
            head = head->next;
        }
        
        // Return default-constructed value (nullptr for pointers)
        return V();
    }
    
    // Alternative: return pointer or optional-like behavior
    bool get(const K& key, V& outValue) const {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                outValue = head->value;
                return true;
            }
            head = head->next;
        }
        
        return false;
    }
    
    void remove(const K& key) {
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
    
    bool contains(const K& key) const {
        int index = hashFunction(key);
        HashNode<K, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                return true;
            }
            head = head->next;
        }
        
        return false;
    }
    
    int getSize() const {
        return size;
    }
    
    bool isEmpty() const {
        return size == 0;
    }
    vector<K> getAllKeys() const {
        vector<K> keys;
        keys.reserve(size);

        for (int i = 0; i < capacity; i++) {
            HashNode<K, V>* curr = table[i];
            while (curr != nullptr) {
                keys.push_back(curr->key);
                curr = curr->next;
            }
        }
        return keys;
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
    
    // Prevent copying (optional, but recommended for resource management)
    MyHashMap(const MyHashMap&) = delete;
    MyHashMap& operator=(const MyHashMap&) = delete;
};

// Specialization for const char* keys
template <typename V>
class MyHashMap<const char*, V> {
private:
    HashNode<const char*, V>** table;
    int capacity;
    int size;
    
    size_t hashFunction(const char* key) const {
        unsigned long hash = 5381;
        for (int i = 0; key[i] != '\0'; i++) {
            hash = ((hash << 5) + hash) + key[i];
        }
        return hash % capacity;
    }
    
    bool keysEqual(const char* k1, const char* k2) const {
        return strcmp(k1, k2) == 0;
    }

public:
    MyHashMap(int cap = 100) : capacity(cap), size(0) {
        table = new HashNode<const char*, V>*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = nullptr;
        }
    }
    
    void insert(const char* key, const V& value) {
        int index = hashFunction(key);
        
        HashNode<const char*, V>* current = table[index];
        while (current != nullptr) {
            if (keysEqual(current->key, key)) {
                current->value = value;
                return;
            }
            current = current->next;
        }
        
        HashNode<const char*, V>* node = new HashNode<const char*, V>(key, value);
        node->next = table[index];
        table[index] = node;
        size++;
    }
    
    V get(const char* key) const {
        int index = hashFunction(key);
        HashNode<const char*, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                return head->value;
            }
            head = head->next;
        }
        
        // Return default-constructed value (nullptr for pointers)
        return V();
    }
    
    bool get(const char* key, V& outValue) const {
        int index = hashFunction(key);
        HashNode<const char*, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                outValue = head->value;
                return true;
            }
            head = head->next;
        }
        
        return false;
    }
    
    void remove(const char* key) {
        int index = hashFunction(key);
        HashNode<const char*, V>* head = table[index];
        HashNode<const char*, V>* prev = nullptr;
        
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
    
    bool contains(const char* key) const {
        int index = hashFunction(key);
        HashNode<const char*, V>* head = table[index];
        
        while (head != nullptr) {
            if (keysEqual(head->key, key)) {
                return true;
            }
            head = head->next;
        }
        
        return false;
    }
    
    int getSize() const {
        return size;
    }
    
    bool isEmpty() const {
        return size == 0;
    }
    vector<const char*> getAllKeys() const {
        vector<const char*> keys;
        keys.reserve(size);

        for (int i = 0; i < capacity; i++) {
            HashNode<const char*, V>* curr = table[i];
            while (curr != nullptr) {
                keys.push_back(curr->key);
                curr = curr->next;
            }
        }
        return keys;
    }

    ~MyHashMap() {
        for (int i = 0; i < capacity; i++) {
            HashNode<const char*, V>* head = table[i];
            while (head != nullptr) {
                HashNode<const char*, V>* temp = head;
                head = head->next;
                delete temp;
            }
        }
        delete[] table;
    }
    
    MyHashMap(const MyHashMap&) = delete;
    MyHashMap& operator=(const MyHashMap&) = delete;
};

#endif
