#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include "OrderQueue.h"
#include "../storage/DiskTypes.h"

const int MAX_KEYS = 5;

struct BTreeNode {
    int numKeys;
    double keys[MAX_KEYS];
    OrderQueue* queues[MAX_KEYS];     // queues of DiskOffset
    BTreeNode* children[MAX_KEYS + 1];
    bool isLeaf;

    BTreeNode(bool leaf) : numKeys(0), isLeaf(leaf) {
        for (int i = 0; i < MAX_KEYS; i++) queues[i] = nullptr;
        for (int i = 0; i <= MAX_KEYS; i++) children[i] = nullptr;
    }
};

#endif
