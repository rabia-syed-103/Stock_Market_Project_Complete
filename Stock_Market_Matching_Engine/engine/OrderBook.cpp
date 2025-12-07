#include "OrderBook.h"
#include <iostream>

OrderBook::OrderBook(std::string sym)
{
    symbol = sym;
    buyTree = new BTree(3);  // degree = 3
    sellTree = new BTree(3);
}

vector<Trade> OrderBook::addOrder(Order* order) {
    std::vector<Trade> trades;

    bool isBuy = order->getSide();

    
    if (isBuy) {
        Order* bestSell = sellTree->getBest();

        while (bestSell != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestSell->price <= order->price) {

            int matchedQty = std::min(order->remainingQty, bestSell->remainingQty);

            Trade trade(
                100 + trades.size(),
                order,
                bestSell,
                matchedQty,
                bestSell->price   // SELL price wins
            );

            trades.push_back(trade);

            order->fill(matchedQty);
            bestSell->fill(matchedQty);

            // remove fully filled sell order
            if (bestSell->isFilled()) {
                sellTree->search(bestSell->price)->dequeue();
            }

            bestSell = sellTree->getBest();
        }

        if (!order->isFilled()) {
            buyTree->insert(order->price, order);
        }
    }

    
    else {
        Order* bestBuy = buyTree->getBest();

        while (bestBuy != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestBuy->price >= order->price) {

            int matchedQty = std::min(order->remainingQty, bestBuy->remainingQty);

            Trade trade(
                200 + trades.size(),
                bestBuy,
                order,
                matchedQty,
                bestBuy->price // BUY price wins
            );

            trades.push_back(trade);

            order->fill(matchedQty);
            bestBuy->fill(matchedQty);

            if (bestBuy->isFilled()) {
                buyTree->search(bestBuy->price)->dequeue();
            }

            bestBuy = buyTree->getBest();
        }

        if (!order->isFilled()) {
            sellTree->insert(order->price, order);
        }
    }

    return trades;
}


Order* OrderBook::getBestBid() {
    Order* best = buyTree->getBest();
    return best;
}

Order* OrderBook::getBestAsk() {
    Order* best = sellTree->getBest();
    return best;
}


void OrderBook::printOrderBook() {
    std::cout << "\n===== ORDER BOOK (" << symbol << ") =====\n";
    std::cout << "--- BUY SIDE ---\n";
    buyTree->print();

    std::cout << "--- SELL SIDE ---\n";
    sellTree->print();
}

void OrderBook::cancelOrder(int orderID) {
    bool found = false;

    // Check BUY tree
    for (double price = buyTree->getLowestKey();
         price <= buyTree->getHighestKey();
         price = buyTree->nextKey(price))
    {
        OrderQueue* q = buyTree->search(price);
        if (!q) continue;

        Order* ord = q->removeOrder(orderID);
        if (ord) {
            ord->cancel();
            std::cout << "Cancelled OrderID " << orderID << " from BUY side\n";
            found = true;
            break;
        }
    }

    if (found) return;

    // Check SELL tree
    for (double price = sellTree->getLowestKey();
         price <= sellTree->getHighestKey();
         price = sellTree->nextKey(price))
    {
        OrderQueue* q = sellTree->search(price);
        if (!q) continue;

        Order* ord = q->removeOrder(orderID);
        if (ord) {
            ord->cancel();
            std::cout << "Cancelled OrderID " << orderID << " from SELL side\n";
            found = true;
            break;
        }
    }

    if (!found)
        std::cout << "OrderID " << orderID << " not found. Cancel failed.\n";
}
