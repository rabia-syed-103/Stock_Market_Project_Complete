#include "OrderBook.h"
#include "../storage/OrderStorage.h"
#include <iostream>
#include <algorithm>

using namespace std;

// External persistent storage
extern OrderStorage orderStorage;

//OrderStorage orderStorage;


OrderBook::OrderBook(std::string sym, OrderStorage& _order ): symbol(sym),orderStorage(_order) {
    buyTree = new BTree(3);   // degree = 3
    sellTree = new BTree(3);
    pthread_mutex_init(&bookLock, NULL);
}

OrderBook::~OrderBook() {

    if (buyTree) {
        delete buyTree;
        buyTree = nullptr;
    }
    if (sellTree) {
        delete sellTree;
        sellTree = nullptr;
    }

    pthread_mutex_destroy(&bookLock);
}

// Add order fully persistent
vector<Trade> OrderBook::addOrder(Order* order) {
    pthread_mutex_lock(&bookLock);
    vector<Trade> trades;

    // Persist new order first and get its offset
    DiskOffset orderOffset = orderStorage.persist(*order);
    std::cerr << "[DBG] addOrder: orderID=" << order->orderID
            << " side=" << order->side << " price=" << order->price
            << " qty=" << order->getRemainingQuantity()
            << " offset=" << orderOffset << "\n";
    if (orderOffset == 0) std::cerr << "[ERR] persist returned 0\n";

    bool isBuy = order->getSide();

    if (isBuy) {
        DiskOffset bestSellOffset = sellTree->getBestSell();

     std::cerr << "[DBG] addOrder: initial bestSellOffset=" << bestSellOffset << "\n";
    while (bestSellOffset != 0 && order->getRemainingQuantity() > 0) {
        Order bestSell = orderStorage.load(bestSellOffset);
        std::cerr << "[DBG] compare: incoming(orderID="<<order->orderID<<",price="<<order->price<<")"
                << " vs bestSell(offset="<<bestSellOffset<<",orderID="<<bestSell.orderID
                <<",price="<<bestSell.price<<",rem="<<bestSell.getRemainingQuantity()<<")\n";

    
            if (bestSell.price > order->price) break;

            if (order->userID == bestSell.userID) {
                double nextPrice = sellTree->nextKey(bestSell.price);
                bestSellOffset = (nextPrice != -1) ? sellTree->search(nextPrice)->peek() : 0;
                if (!bestSellOffset) break;
                continue;
            }

            int matchedQty = std::min(order->getRemainingQuantity(), bestSell.getRemainingQuantity());
                    // after a match:
            std::cerr << "[DBG] matched qty="<<matchedQty<<" updating disk offsets order="<<orderOffset<<" bestSell="<<bestSellOffset<<"\n";
        // before dequeue:
            // Use copies, never pointers
            Order incomingCopy = *order;
            Order counterCopy = bestSell;

            trades.emplace_back(
                100 + trades.size(),
                incomingCopy,
                counterCopy,
                matchedQty,
                counterCopy.price
            );

            // Update quantities and statuses
            order->reduceRemainingQty(matchedQty);
            bestSell.reduceRemainingQty(matchedQty);

            order->status = (order->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";
            bestSell.status = (bestSell.getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";

            // Save updated orders to disk
            orderStorage.save(*order, orderOffset);
            orderStorage.save(bestSell, bestSellOffset);
            std::cerr << "[DBG] about to dequeue price="<<bestSell.price<<"\n";
            // Remove filled sell orders from tree
            if (bestSell.getRemainingQuantity() == 0)
                sellTree->search(bestSell.price)->dequeue();

            bestSellOffset = sellTree->getBestSell();
        }

        if (order->getRemainingQuantity() > 0)
            buyTree->insert(order->price, orderOffset);

    } else {
        DiskOffset bestBuyOffset = buyTree->getBest();

        while (bestBuyOffset != 0 && order->getRemainingQuantity() > 0) {
            Order bestBuy = orderStorage.load(bestBuyOffset);

            if (bestBuy.price < order->price) break;

            if (order->userID == bestBuy.userID) {
                double prevPrice = buyTree->prevKey(bestBuy.price);
                bestBuyOffset = (prevPrice != -1) ? buyTree->search(prevPrice)->peek() : 0;
                if (!bestBuyOffset) break;
                continue;
            }

            int matchedQty = std::min(order->getRemainingQuantity(), bestBuy.getRemainingQuantity());

            Order incomingCopy = *order;
            Order counterCopy = bestBuy;

            trades.emplace_back(
                200 + trades.size(),
                counterCopy,
                incomingCopy,
                matchedQty,
                counterCopy.price
            );

            order->reduceRemainingQty(matchedQty);
            bestBuy.reduceRemainingQty(matchedQty);

            order->status = (order->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";
            bestBuy.status = (bestBuy.getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";

            orderStorage.save(*order, orderOffset);
            orderStorage.save(bestBuy, bestBuyOffset);

            if (bestBuy.getRemainingQuantity() == 0)
                buyTree->search(bestBuy.price)->dequeue();

            bestBuyOffset = buyTree->getBest();
        }

        if (order->getRemainingQuantity() > 0)
            sellTree->insert(order->price, orderOffset);
    }

    pthread_mutex_unlock(&bookLock);
    return trades;
}



// Cancel order fully persistent
void OrderBook::cancelOrder(int orderID) {
    pthread_mutex_lock(&bookLock);
    bool found = false;

    // Check BUY tree
    double price = buyTree->getLowestKey();
    double highest = buyTree->getHighestKey();
    while (price != -1 && price <= highest) {
        OrderQueue* q = buyTree->search(price);
        if (q && q->getSize() > 0) {
            DiskOffset off = q->removeOrder(orderID, orderStorage);
            if (off != 0) {
                Order o = orderStorage.load(off);
                o.cancel();
                orderStorage.save(o, off);
                cout << "Cancelled OrderID " << orderID << " from BUY side\n";
                found = true;
                break;
            }
        }
        double next = buyTree->nextKey(price);
        if (next == -1 || next == price) break;
        price = next;
    }

    // Check SELL tree
    if (!found) {
        price = sellTree->getLowestKey();
        highest = sellTree->getHighestKey();
        while (price != -1 && price <= highest) {
            OrderQueue* q = sellTree->search(price);
            if (q && q->getSize() > 0) {
                DiskOffset off = q->removeOrder(orderID, orderStorage);
                if (off != 0) {
                    Order o = orderStorage.load(off);
                    o.cancel();
                    orderStorage.save(o, off);
                    cout << "Cancelled OrderID " << orderID << " from SELL side\n";
                    found = true;
                    break;
                }
            }
            double next = sellTree->nextKey(price);
            if (next == -1 || next == price) break;
            price = next;
        }
    }

    if (!found)
        cout << "OrderID " << orderID << " not found. Cancel failed.\n";

    pthread_mutex_unlock(&bookLock);
}

// Get best bid fully persistent
Order OrderBook::getBestBid() {
    pthread_mutex_lock(&bookLock);
    DiskOffset off = buyTree->getBest();
    Order o;
    if (off != 0) o = orderStorage.load(off);
    pthread_mutex_unlock(&bookLock);
    return o;
}

// Get best ask fully persistent
Order OrderBook::getBestAsk() {
    pthread_mutex_lock(&bookLock);
    DiskOffset off = sellTree->getBestSell();
    Order o;
    if (off != 0) o = orderStorage.load(off);
    pthread_mutex_unlock(&bookLock);
    return o;
}

// Print order book fully persistent
void OrderBook::printOrderBook() {
    pthread_mutex_lock(&bookLock);

    cout << "\nORDER BOOK (" << symbol << ")\n";

    // BUY SIDE
    cout << "BUY SIDE\n";
    double price = buyTree->getHighestKey();
    double lowest = buyTree->getLowestKey();
    while (price != -1 && price >= lowest) {
        OrderQueue* q = buyTree->search(price);
        if (q && q->getSize() > 0) q->printQueue(orderStorage);
        double prev = buyTree->prevKey(price);
        if (prev == price || prev == -1) break;
        price = prev;
    }

    // SELL SIDE
    cout << "SELL SIDE\n";
    price = sellTree->getLowestKey();
    double highest = sellTree->getHighestKey();
    while (price != -1 && price <= highest) {
        OrderQueue* q = sellTree->search(price);
        if (q && q->getSize() > 0) q->printQueue(orderStorage);
        double next = sellTree->nextKey(price);
        if (next == price || next == -1) break;
        price = next;
    }

    pthread_mutex_unlock(&bookLock);
}

string OrderBook::getSymbol() const {
    return symbol;
}

void OrderBook::rebuildFromStorage() {
    pthread_mutex_lock(&bookLock);

    // Clear existing trees (if needed)
    delete buyTree;
    delete sellTree;
    buyTree = new BTree(3);
    sellTree = new BTree(3);

    // Load all orders from storage
    std::vector<Order> allOrders = orderStorage.loadAllOrdersForSymbol(symbol);

    for (auto& o : allOrders) {
        if (o.status == "FILLED" || o.status == "CANCELLED") continue; // skip inactive

        DiskOffset offset = orderStorage.getOffsetForOrder(o.orderID);

        if (o.getSide()) { // BUY
            buyTree->insert(o.price, offset);
        } else {           // SELL
            sellTree->insert(o.price, offset);
        }
    }

    pthread_mutex_unlock(&bookLock);
}

