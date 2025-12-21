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

    //rebuildFromStorage();
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
    
    if (orderOffset == 0) {
        std::cerr << "[ERR] persist returned 0\n";
        pthread_mutex_unlock(&bookLock);
        return trades;
    }

    bool isBuy = order->getSide();

    if (isBuy) {
        // BUY order - match against SELL tree
        DiskOffset bestSellOffset = sellTree->getBestSell();
        std::cerr << "[DBG] addOrder: initial bestSellOffset=" << bestSellOffset << "\n";
        
        while (bestSellOffset != 0 && order->getRemainingQuantity() > 0) {
            Order bestSell = orderStorage.load(bestSellOffset);
            
            std::cerr << "[DBG] compare: incoming(orderID=" << order->orderID 
                      << ",price=" << order->price << ")"
                      << " vs bestSell(offset=" << bestSellOffset 
                      << ",orderID=" << bestSell.orderID
                      << ",price=" << bestSell.price 
                      << ",rem=" << bestSell.getRemainingQuantity() << ")\n";

            // Check if price matches
            if (bestSell.price > order->price) {
                std::cerr << "[DBG] Price mismatch, stopping match\n";
                break;
            }

            // Self-match prevention
            if (order->userID == bestSell.userID) {
                std::cerr << "[DBG] Self-match detected, skipping\n";
                double nextPrice = sellTree->nextKey(bestSell.price);
                bestSellOffset = (nextPrice != -1) ? sellTree->search(nextPrice)->peek() : 0;
                if (!bestSellOffset) break;
                continue;
            }

            // Calculate matched quantity
            int matchedQty = std::min(order->getRemainingQuantity(), 
                                     bestSell.getRemainingQuantity());
            
            std::cerr << "[DBG] matched qty=" << matchedQty 
                      << " updating disk offsets order=" << orderOffset 
                      << " bestSell=" << bestSellOffset << "\n";

            // Create trade (use copies to avoid pointer issues)
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

            std::cerr << "[DBG] After match: order->rem=" << order->getRemainingQuantity()
                      << " bestSell.rem=" << bestSell.getRemainingQuantity() << "\n";

            // Save updated orders to disk
            orderStorage.save(*order, orderOffset);
            orderStorage.save(bestSell, bestSellOffset);

            // ✅ ALWAYS dequeue the matched order
            OrderQueue* sellQueue = sellTree->search(bestSell.price);
            if (sellQueue) {
                sellQueue->dequeue();
                std::cerr << "[DBG] Dequeued sell order, queue size now=" 
                          << sellQueue->getSize() << "\n";
            }

            // ✅ If counter order still has quantity, re-insert it
            if (bestSell.getRemainingQuantity() > 0) {
                std::cerr << "[DBG] Counter order partial fill, re-inserting offset=" 
                          << bestSellOffset << "\n";
                sellTree->insert(bestSell.price, bestSellOffset);
            }

            // Get next best sell order
            bestSellOffset = sellTree->getBestSell();
            std::cerr << "[DBG] Next bestSellOffset=" << bestSellOffset << "\n";
        }

        // If incoming order has remaining quantity, add to buy tree
        if (order->getRemainingQuantity() > 0) {
            std::cerr << "[DBG] Incoming order has remaining qty=" 
                      << order->getRemainingQuantity() << ", adding to buy tree\n";
            buyTree->insert(order->price, orderOffset);
        }

    } else {
        // SELL order - match against BUY tree
        DiskOffset bestBuyOffset = buyTree->getBest();
        std::cerr << "[DBG] addOrder: initial bestBuyOffset=" << bestBuyOffset << "\n";

        while (bestBuyOffset != 0 && order->getRemainingQuantity() > 0) {
            Order bestBuy = orderStorage.load(bestBuyOffset);
            
            std::cerr << "[DBG] compare: incoming(orderID=" << order->orderID 
                      << ",price=" << order->price << ")"
                      << " vs bestBuy(offset=" << bestBuyOffset 
                      << ",orderID=" << bestBuy.orderID
                      << ",price=" << bestBuy.price 
                      << ",rem=" << bestBuy.getRemainingQuantity() << ")\n";

            // Check if price matches
            if (bestBuy.price < order->price) {
                std::cerr << "[DBG] Price mismatch, stopping match\n";
                break;
            }

            // Self-match prevention
            if (order->userID == bestBuy.userID) {
                std::cerr << "[DBG] Self-match detected, skipping\n";
                double prevPrice = buyTree->prevKey(bestBuy.price);
                bestBuyOffset = (prevPrice != -1) ? buyTree->search(prevPrice)->peek() : 0;
                if (!bestBuyOffset) break;
                continue;
            }

            // Calculate matched quantity
            int matchedQty = std::min(order->getRemainingQuantity(), 
                                     bestBuy.getRemainingQuantity());
            
            std::cerr << "[DBG] matched qty=" << matchedQty 
                      << " updating disk offsets order=" << orderOffset 
                      << " bestBuy=" << bestBuyOffset << "\n";

            // Create trade (use copies)
            Order incomingCopy = *order;
            Order counterCopy = bestBuy;

            trades.emplace_back(
                200 + trades.size(),
                counterCopy,
                incomingCopy,
                matchedQty,
                counterCopy.price
            );

            // Update quantities and statuses
            order->reduceRemainingQty(matchedQty);
            bestBuy.reduceRemainingQty(matchedQty);

            order->status = (order->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";
            bestBuy.status = (bestBuy.getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";

            std::cerr << "[DBG] After match: order->rem=" << order->getRemainingQuantity()
                      << " bestBuy.rem=" << bestBuy.getRemainingQuantity() << "\n";

            // Save updated orders to disk
            orderStorage.save(*order, orderOffset);
            orderStorage.save(bestBuy, bestBuyOffset);

            // ✅ ALWAYS dequeue the matched order
            OrderQueue* buyQueue = buyTree->search(bestBuy.price);
            if (buyQueue) {
                buyQueue->dequeue();
                std::cerr << "[DBG] Dequeued buy order, queue size now=" 
                          << buyQueue->getSize() << "\n";
            }

            // ✅ If counter order still has quantity, re-insert it
            if (bestBuy.getRemainingQuantity() > 0) {
                std::cerr << "[DBG] Counter order partial fill, re-inserting offset=" 
                          << bestBuyOffset << "\n";
                buyTree->insert(bestBuy.price, bestBuyOffset);
            }

            // Get next best buy order
            bestBuyOffset = buyTree->getBest();
            std::cerr << "[DBG] Next bestBuyOffset=" << bestBuyOffset << "\n";
        }

        // If incoming order has remaining quantity, add to sell tree
        if (order->getRemainingQuantity() > 0) {
            std::cerr << "[DBG] Incoming order has remaining qty=" 
                      << order->getRemainingQuantity() << ", adding to sell tree\n";
            sellTree->insert(order->price, orderOffset);
        }
    }
std::cerr << "[DBG] addOrder: about to unlock bookLock\n";
    pthread_mutex_unlock(&bookLock);
    
    std::cerr << "[DBG] addOrder: unlocked successfully\n";
    std::cerr << "[DBG] addOrder complete: " << trades.size() << " trades executed\n";
    return trades;
    cout << "Still here!";
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
    
    vector<Order> allOrders = orderStorage.loadAllOrdersForSymbol(symbol);
    
    // ✅ ADD: Don't rebuild if no orders
    if (allOrders.empty()) {
        pthread_mutex_unlock(&bookLock);
        return;  // Don't print anything, just skip
    }
    
    delete buyTree;
    delete sellTree;
    buyTree = new BTree(3);
    sellTree = new BTree(3);

    for (const Order& o : allOrders) {
        if (o.status != "ACTIVE" && o.status != "PARTIAL_FILL") {
            continue;
        }
        
        if (o.getRemainingQuantity() <= 0) {
            continue;
        }

        DiskOffset offset = orderStorage.getOffsetForOrder(o.orderID);
        if (offset == 0) continue;

        if (o.getSide()) {
            buyTree->insert(o.price, offset);
        } else {
            sellTree->insert(o.price, offset);
        }
    }

    pthread_mutex_unlock(&bookLock);
    
    cout << "Rebuilt order book for " << symbol << " with " 
         << allOrders.size() << " orders from storage.\n";
}

Order OrderBook::loadOrderFromStorage(int orderID) {
    return orderStorage.loadOrder(orderID);
}

