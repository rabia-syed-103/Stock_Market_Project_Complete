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
/*
Order* BTree::getBest() {
    if (!root || root->numKeys == 0) return nullptr;
    
    BTreeNode* node = root;
    while (!node->isLeaf) {
        node = node->children[node->numKeys]; 
    }
    
    // Use for loop instead of while to prevent infinite loop
    for (int i = node->numKeys - 1; i >= 0; i--) {
        OrderQueue* q = node->queues[i];
        
        if (q && q->getSize() > 0) {
            Order* o1 = q->peek();
            return o1;
        }
    }
    
    return nullptr;
}
*/

Order* BTree::getBest() {
    if (!root || root->numKeys == 0) return nullptr;

    double price = getHighestKey();
    double lowest = getLowestKey();
    if (price == -1 || lowest == -1) return nullptr;

    // Scan from highest down to lowest using prevKey
    while (price != -1 && price >= lowest) {
        OrderQueue* q = search(price);
        if (q && q->getSize() > 0) {
            // Clean up any zero-remaining orders at front
            while (q->getSize() > 0) {
                Order* o = q->peek();
                if (!o) { q->dequeue(); continue; }
                if (o->getRemainingQuantity() <= 0) {
                    q->dequeue(); // remove exhausted order
                    continue;
                }
                // Found valid top-of-queue order
                return o;
            }
            // queue exhausted, continue scanning to next lower price
        }
        double prev = prevKey(price);
        if (prev == price) break;
        price = prev;
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

/*
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

        if (curr->isLeaf) break; 
        curr = curr->children[i]; 
    }

    return successor;
}
*/
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

/*
Order* BTree::getBestSell() {
    if (!root) return nullptr;
    
    // Get lowest price in tree
    double lowestPrice = getLowestKey();
    if (lowestPrice == -1) return nullptr;
    
    // Get the queue at that price
    OrderQueue* queue = search(lowestPrice);
    if (!queue || queue->getSize() == 0) return nullptr;
    
    // Return first order in queue (FIFO)
    return queue->peek();
}

*/

Order* BTree::getBestSell() {
    if (!root || root->numKeys == 0) return nullptr;

    double price = getLowestKey();
    double highest = getHighestKey();
    if (price == -1 || highest == -1) return nullptr;

    // Scan from lowest up to highest using nextKey
    while (price != -1 && price <= highest) {
        OrderQueue* q = search(price);
        if (q && q->getSize() > 0) {
            // Clean up any zero-remaining orders at front
            while (q->getSize() > 0) {
                Order* o = q->peek();
                if (!o) { q->dequeue(); continue; }
                if (o->getRemainingQuantity() <= 0) {
                    q->dequeue(); // remove exhausted order
                    continue;
                }
                // Found valid top-of-queue order
                return o;
            }
            // queue exhausted, continue scanning
        }
        double nxt = nextKey(price);
        if (nxt == price) break;
        price = nxt;
    }

    return nullptr;
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