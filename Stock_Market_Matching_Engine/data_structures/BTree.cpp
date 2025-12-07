// btree.cpp
#include "BTree.h"
#include <iostream>
using namespace std;

BTree::BTree(int _t) {
    root = new BTreeNode(true);
    t = _t;
}

// Traverse for debugging
void BTree::traverse(BTreeNode* node) {
    if (!node) return;
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) traverse(node->children[i]);
        cout << node->keys[i] << " ";
    }
    if (!node->isLeaf) traverse(node->children[i]);
}

void BTree::print() {
    traverse(root);
    cout << endl;
}

// Search for price
BTreeNode* BTree::search(BTreeNode* node, double key) {
    int i = 0;
    while (i < node->numKeys && key > node->keys[i]) i++;

    if (i < node->numKeys && node->keys[i] == key) return node;

    if (node->isLeaf) return nullptr;

    return search(node->children[i], key);
}

OrderQueue* BTree::search(double key) {
    BTreeNode* node = search(root, key);
    if (!node) return nullptr;
    for (int i = 0; i < node->numKeys; i++)
        if (node->keys[i] == key) return node->queues[i];
    return nullptr;
}

// Insert
void BTree::insert(double key, Order* order) {
    BTreeNode* r = root;
    if (r->numKeys == MAX_KEYS) {
        BTreeNode* s = new BTreeNode(false);
        s->children[0] = r;
        root = s;
        splitChild(s, 0, r);
        insertNonFull(s, key, order);
    } else {
        insertNonFull(r, key, order);
    }
}

// Insert in non-full node
void BTree::insertNonFull(BTreeNode* node, double key, Order* order) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        // find location
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i+1] = node->keys[i];
            node->queues[i+1] = node->queues[i];
            i--;
        }

        // same price exists?
        if (i >= 0 && node->keys[i] == key) {
            node->queues[i]->enqueue(order);
        } else {
            node->keys[i+1] = key;
            node->queues[i+1] = new OrderQueue();
            node->queues[i+1]->enqueue(order);
            node->numKeys++;
        }
    } else {
        while (i >= 0 && node->keys[i] > key) i--;
        i++;
        if (node->children[i]->numKeys == MAX_KEYS) {
            splitChild(node, i, node->children[i]);
            if (node->keys[i] < key) i++;
        }
        insertNonFull(node->children[i], key, order);
    }
}

// Split child
void BTree::splitChild(BTreeNode* parent, int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->isLeaf);
    int mid = MAX_KEYS/2;
    z->numKeys = MAX_KEYS - mid - 1;

    for (int j=0;j<z->numKeys;j++) {
        z->keys[j] = y->keys[j + mid + 1];
        z->queues[j] = y->queues[j + mid + 1];
    }

    if (!y->isLeaf) {
        for (int j=0;j<=z->numKeys;j++) {
            z->children[j] = y->children[j + mid + 1];
        }
    }

    y->numKeys = mid;

    for (int j=parent->numKeys;j>i;j--) {
        parent->children[j+1] = parent->children[j];
        parent->keys[j] = parent->keys[j-1];
        parent->queues[j] = parent->queues[j-1];
    }

    parent->children[i+1] = z;
    parent->keys[i] = y->keys[mid];
    parent->queues[i] = y->queues[mid];
    parent->numKeys++;
}

// Get best price (for buy: max, for sell: min)
Order* BTree::getBest() {
    BTreeNode* node = root;
    while (!node->isLeaf) {
        node = node->children[node->numKeys]; // rightmost for max
    }
    if (node->numKeys == 0) return nullptr;
    return node->queues[node->numKeys-1]->peek();
}
