#ifndef HASHNODE_H
#define HASHNODE_H

template<typename K, typename V>
class HashNode {
public:
    K key;
    V value;
    HashNode* next;
    
    HashNode(K k, V v) : key(k), value(v), next(nullptr) {}
};

#endif