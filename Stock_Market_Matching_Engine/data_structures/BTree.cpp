// btree.cpp
#include "BTree.h"
#include <iostream>
using namespace std;

BTree::BTree(int _t) {
    root = new BTreeNode(true);
    t = _t;
}

void BTree::traverse(BTreeNode* node) {
    if (!node) return;
    int i;
    for (i = 0; i < node->numKeys; i++) {
        if (!node->isLeaf) traverse(node->children[i]);
        
        // Only print if queue has orders
        if (node->queues[i] && node->queues[i]->getSize() > 0) {
            cout << node->keys[i] << " ";
        }
    }
    if (!node->isLeaf) traverse(node->children[i]);
}

void BTree::print() {
    traverse(root);
    cout << endl;
}

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

void BTree::insertNonFull(BTreeNode* node, double key, Order* order) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i+1] = node->keys[i];
            node->queues[i+1] = node->queues[i];
            i--;
        }

        
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

void BTree::splitChild(BTreeNode* parent, int i, BTreeNode* y) {
    BTreeNode* z = new BTreeNode(y->isLeaf);
    int mid = MAX_KEYS / 2;
    z->numKeys = MAX_KEYS - mid - 1;

    // Copy keys and queues from y to z
    for (int j = 0; j < z->numKeys; j++) {
        z->keys[j] = y->keys[j + mid + 1];
        z->queues[j] = y->queues[j + mid + 1];
        
        // ✅ IMPORTANT: Clear the original pointers in y
        // This prevents double-free when y is deleted later
        y->queues[j + mid + 1] = nullptr;
    }

    // Copy children if not leaf
    if (!y->isLeaf) {
        for (int j = 0; j <= z->numKeys; j++) {
            z->children[j] = y->children[j + mid + 1];
            y->children[j + mid + 1] = nullptr;  // ✅ Clear original pointers
        }
    }

    // Update y's key count (it now only has 'mid' keys)
    y->numKeys = mid;

    // Make space in parent for new child
    for (int j = parent->numKeys; j > i; j--) {
        parent->children[j + 1] = parent->children[j];
        parent->keys[j] = parent->keys[j - 1];
        
        // ✅ FIXED: Don't overwrite existing queue, shift it
        parent->queues[j] = parent->queues[j - 1];
    }

    // Insert middle key from y into parent
    parent->children[i + 1] = z;
    parent->keys[i] = y->keys[mid];
    
    // ✅ FIXED: Don't overwrite - move the middle queue up
    // Note: In a typical B-Tree for range queries, we might not store
    // the middle key's queue in parent. But if we do:
    parent->queues[i] = y->queues[mid];
    y->queues[mid] = nullptr;  // ✅ Clear to prevent double-free
    
    parent->numKeys++;
}

Order* BTree::getBest() {
    if (!root || root->numKeys == 0) return nullptr;
    
    BTreeNode* node = root;
    while (!node->isLeaf) {
        node = node->children[node->numKeys]; 
    }
    
    // Search backwards through the leaf
    while (node && node->numKeys > 0) {
        // Check last price level
        int lastIdx = node->numKeys - 1;
        OrderQueue* q = node->queues[lastIdx];
        
        if (q && q->getSize() > 0) {
            return q->peek();
        }
        
        // This queue is empty - should remove this key from tree
        // For now, just skip it (proper fix: remove key from B-Tree)
        node->numKeys--;  // Temporary fix: just hide this key
    }
    
    return nullptr;
}

double BTree::getLowestKey() {
    if (!root || root->numKeys == 0) return -1;

    BTreeNode* curr = root;
    while (!curr->isLeaf) {
        curr = curr->children[0]; // leftmost child
    }

    return curr->keys[0]; // smallest key in leftmost leaf
}

double BTree::getHighestKey() {
    if (!root || root->numKeys == 0) return -1;

    BTreeNode* curr = root;
    while (!curr->isLeaf) {
        curr = curr->children[curr->numKeys]; // rightmost child
    }

    return curr->keys[curr->numKeys - 1]; // largest key in rightmost leaf
}

double BTree::nextKey(double price) {
    if (!root) return -1;

    BTreeNode* curr = root;
    double successor = -1;

    while (curr != nullptr) {
        int i = 0;
        while (i < curr->numKeys && curr->keys[i] <= price) i++;

        if (i < curr->numKeys) {
            successor = curr->keys[i]; // potential next key
        }

        if (curr->isLeaf) break; // reached leaf
        curr = curr->children[i];  // go down the correct child
    }

    return successor;
}

