// btree.h
#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreeNode.h"
using namespace std;

class BTree {
private:
    BTreeNode* root;
    int t; // minimum degree

    void traverse(BTreeNode* node);
    BTreeNode* search(BTreeNode* node, double key);
    void insertNonFull(BTreeNode* node, double key, Order* order);
    void splitChild(BTreeNode* parent, int i, BTreeNode* child);

public:
    BTree(int _t);
    void insert(double key, Order* order);
    OrderQueue* search(double key);
    void print();
    Order* getBest(); // max buy or min sell
};

#endif
