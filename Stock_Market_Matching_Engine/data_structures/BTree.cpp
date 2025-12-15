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
void BTree::insert(double key, DiskOffset offset)
{
    BTreeNode* r = root;
    if (r->numKeys == MAX_KEYS) {
        BTreeNode* s = new BTreeNode(false);
        s->children[0] = r;
        root = s;
        splitChild(s, 0, r);
        insertNonFull(s, key, offset);
    } else {
        insertNonFull(r, key, offset);
    }
}

void BTree::insertNonFull(BTreeNode* node, double key, DiskOffset offset) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        while (i >= 0 && node->keys[i] > key) {
            node->keys[i + 1] = node->keys[i];
            node->queues[i + 1] = node->queues[i];
            i--;
        }

           if (i >= 0 && node->keys[i] == key) {
        std::cerr << "[DBG] BTree::insert: enqueuing offset="<<offset<<" at existing key="<<key<<"\n";
        node->queues[i]->enqueue(offset);
    } else {
            std::cerr << "[DBG] BTree::insert: creating new queue at key="<<key<<" offset="<<offset<<"\n";
            node->keys[i + 1] = key;
            node->queues[i + 1] = new OrderQueue();
            node->queues[i + 1]->enqueue(offset);
            node->numKeys++;
        }
    } else {
        while (i >= 0 && node->keys[i] > key) i--;
        i++;

        if (node->children[i]->numKeys == MAX_KEYS) {
            splitChild(node, i, node->children[i]);
            if (node->keys[i] < key) i++;
        }
        insertNonFull(node->children[i], key, offset);
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
        

        y->queues[j + mid + 1] = nullptr;
    }

    // Copy children if not leaf
    if (!y->isLeaf) {
        for (int j = 0; j <= z->numKeys; j++) {
            z->children[j] = y->children[j + mid + 1];
            y->children[j + mid + 1] = nullptr;  
        }
    }

    // Update y's key count (it now only has 'mid' keys)
    y->numKeys = mid;

    // Make space in parent for new child
    for (int j = parent->numKeys; j > i; j--) {
        parent->children[j + 1] = parent->children[j];
        parent->keys[j] = parent->keys[j - 1];
        
        parent->queues[j] = parent->queues[j - 1];
    }

    // Insert middle key from y into parent
    parent->children[i + 1] = z;
    parent->keys[i] = y->keys[mid];
    
    parent->queues[i] = y->queues[mid];
    y->queues[mid] = nullptr;  
    
    parent->numKeys++;
}

DiskOffset BTree::getBest() {
    if (!root || root->numKeys == 0) return 0;

    double price = getHighestKey();
    double lowest = getLowestKey();

    while (price != -1 && price >= lowest) {
        OrderQueue* q = search(price);
        if (q && q->getSize() > 0) {
            while (q->getSize() > 0) {
                DiskOffset off = q->peek();
                if (off == 0) { q->dequeue(); continue; }
                return off;
            }
        }
        price = prevKey(price);
    }
    return 0;
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
    if (!root || root->numKeys == 0) return -1;

    BTreeNode* curr = root;
    double successor = -1;

    while (curr != nullptr) {
        int i = 0;
        while (i < curr->numKeys && curr->keys[i] <= price) i++;

        if (i < curr->numKeys) {
            successor = curr->keys[i]; // potential next key
        }

        if (curr->isLeaf) break;
        curr = curr->children[i];
    }

    // If successor equals price or wasn't found, return -1
    if (successor <= price) return -1;
    return successor;
}


DiskOffset BTree::getBestSell() {
    if (!root || root->numKeys == 0) return 0;

    double price = getLowestKey();
    double highest = getHighestKey();

    while (price != -1 && price <= highest) {
        OrderQueue* q = search(price);
        if (q && q->getSize() > 0) {
            while (q->getSize() > 0) {
                DiskOffset off = q->peek();
                if (off == 0) { q->dequeue(); continue; }
                return off;
            }
        }
        price = nextKey(price);
    }
    return 0;
}

double BTree::prevKey(double price) {
    if (!root) return -1;

    BTreeNode* curr = root;
    double predecessor = -1;

    while (curr != nullptr) {
        int i = curr->numKeys - 1;
        
        // Find largest key that is < price
        while (i >= 0 && curr->keys[i] >= price) i--;

        if (i >= 0) {
            predecessor = curr->keys[i];  // potential prev key
        }

        if (curr->isLeaf) break;
        
        // Go to appropriate child
        if (i >= 0) {
            curr = curr->children[i + 1];
        } else {
            curr = curr->children[0];
        }
    }

    return predecessor;
}

static void freeBTreeNode(BTreeNode* node) {
    if (!node) return;
    // delete queues owned by this node
    for (int i = 0; i < node->numKeys; ++i) {
        if (node->queues[i]) {
            delete node->queues[i];
            node->queues[i] = nullptr;
        }
    }
    // if not leaf, recurse children
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; ++i) {
            if (node->children[i]) {
                freeBTreeNode(node->children[i]);
                node->children[i] = nullptr;
            }
        }
    }
    delete node;
}

BTree::~BTree() {
    if (root) {
        freeBTreeNode(root);
        root = nullptr;
    }
}

