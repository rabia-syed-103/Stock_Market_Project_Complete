#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <string>
#include <vector>
#include "../core/Order.h"
#include "../core/Trade.h"
#include "../data_structures/BTree.h"
#include "../data_structures/OrderQueue.h"

using namespace std;

class OrderBook {
private:
    std::string symbol;
    BTree* buyTree;   // stores BUY prices (max best)
    BTree* sellTree;  // stores SELL prices (min best)

public:
    OrderBook(std::string sym);

    vector<Trade> addOrder(Order* order); // Returns trades when matched
    void cancelOrder(int orderID);
    Order* getBestBid(); // highest BUY
    Order* getBestAsk(); // lowest SELL
    void printOrderBook();
};

#endif


