#ifndef BTREE_H
#define BTREE_H

#include "BTreeNode.h"
#include "../storage/DiskTypes.h"

class BTree {
private:
    BTreeNode* root;
    int t;

    void traverse(BTreeNode* node);
    BTreeNode* search(BTreeNode* node, double key);
    void insertNonFull(BTreeNode* node, double key, DiskOffset offset);
    void splitChild(BTreeNode* parent, int i, BTreeNode* child);

public:
    BTree(int _t);

    void insert(double key, DiskOffset offset);
    OrderQueue* search(double key);

    void print();

    DiskOffset getBest();       // BUY side
    DiskOffset getBestSell();   // SELL side

    double getLowestKey();
    double getHighestKey();
    double nextKey(double price);
    double prevKey(double price);
    //static void freeBTreeNode(BTreeNode* node);
    
    void removeOrder(double price, DiskOffset offset);
    ~BTree();
};

#endif
