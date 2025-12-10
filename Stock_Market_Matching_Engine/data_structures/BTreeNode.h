// btree_node.h
#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include "OrderQueue.h"

const int MAX_KEYS = 5; // max keys per node 

struct BTreeNode {
    int numKeys;                   // current keys
    double keys[MAX_KEYS];          // prices
    OrderQueue* queues[MAX_KEYS];   // queues at each price
    BTreeNode* children[MAX_KEYS + 1]; // child pointers
    bool isLeaf;

    BTreeNode(bool leaf) {
        numKeys = 0;
        isLeaf = leaf;
        for(int i=0;i<MAX_KEYS;i++)
            queues[i] = nullptr;
        for(int i=0;i<=MAX_KEYS;i++)
            children[i] = nullptr;
    }
};

#endif
