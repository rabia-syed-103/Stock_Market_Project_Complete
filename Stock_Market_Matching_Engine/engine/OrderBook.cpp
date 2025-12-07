#include "OrderBook.h"
#include <iostream>

OrderBook::OrderBook(std::string sym) : symbol(sym) {
    buyTree = new BTree(3);   // degree = 3
    sellTree = new BTree(3);
    
    // Initialize mutex
    pthread_mutex_init(&bookLock, NULL);
}

OrderBook::~OrderBook() {
    //  Cleanup
    delete buyTree;
    delete sellTree;
    pthread_mutex_destroy(&bookLock);
}

vector<Trade> OrderBook::addOrder(Order* order) {
    //  LOCK: Entire matching process must be atomic
    pthread_mutex_lock(&bookLock);
    
    std::vector<Trade> trades;
    bool isBuy = order->getSide();
    
    if (isBuy) {
        // Try to match with SELL orders
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
            
            // Remove fully filled sell order
            if (bestSell->isFilled()) {
                sellTree->search(bestSell->price)->dequeue();
            }
            
            bestSell = sellTree->getBest();
        }
        
        // Add remaining to buy tree
        if (!order->isFilled()) {
            buyTree->insert(order->price, order);
        }
        
    } else {
        // Try to match with BUY orders
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
                bestBuy->price  // BUY price wins
            );
            
            trades.push_back(trade);
            order->fill(matchedQty);
            bestBuy->fill(matchedQty);
            
            if (bestBuy->isFilled()) {
                buyTree->search(bestBuy->price)->dequeue();
            }
            
            bestBuy = buyTree->getBest();
        }
        
        // Add remaining to sell tree
        if (!order->isFilled()) {
            sellTree->insert(order->price, order);
        }
    }
    
    //  UNLOCK before returning
    pthread_mutex_unlock(&bookLock);
    return trades;
}

void OrderBook::cancelOrder(int orderID) {
    //  LOCK: Prevent concurrent modifications
    pthread_mutex_lock(&bookLock);
    
    bool found = false;
    
    // Check BUY tree
    double lowestBuy = buyTree->getLowestKey();
    double highestBuy = buyTree->getHighestKey();
    
    if (lowestBuy != -1 && highestBuy != -1) {
        double price = lowestBuy;
        
        while (price != -1 && price <= highestBuy) {
            OrderQueue* q = buyTree->search(price);
            
            if (q && q->getSize() > 0) {
                Order* ord = q->removeOrder(orderID);
                if (ord) {
                    ord->cancel();
                    std::cout << "Cancelled OrderID " << orderID << " from BUY side\n";
                    found = true;
                    break;
                }
            }
            
            double nextPrice = buyTree->nextKey(price);
            if (nextPrice == -1 || nextPrice == price) break;
            price = nextPrice;
        }
    }
    
    if (!found) {
        // Check SELL tree
        double lowestSell = sellTree->getLowestKey();
        double highestSell = sellTree->getHighestKey();
        
        if (lowestSell != -1 && highestSell != -1) {
            double price = lowestSell;
            
            while (price != -1 && price <= highestSell) {
                OrderQueue* q = sellTree->search(price);
                
                if (q && q->getSize() > 0) {
                    Order* ord = q->removeOrder(orderID);
                    if (ord) {
                        ord->cancel();
                        std::cout << "Cancelled OrderID " << orderID << " from SELL side\n";
                        found = true;
                        break;
                    }
                }
                
                double nextPrice = sellTree->nextKey(price);
                if (nextPrice == -1 || nextPrice == price) break;
                price = nextPrice;
            }
        }
    }
    
    if (!found) {
        std::cout << "OrderID " << orderID << " not found. Cancel failed.\n";
    }
    
    //  UNLOCK
    pthread_mutex_unlock(&bookLock);
}

Order* OrderBook::getBestBid() {
    //  LOCK for read
    pthread_mutex_lock(&bookLock);
    Order* best = buyTree->getBest();
    pthread_mutex_unlock(&bookLock);
    return best;
}

Order* OrderBook::getBestAsk() {
    //  LOCK for read
    pthread_mutex_lock(&bookLock);
    Order* best = sellTree->getBest();
    pthread_mutex_unlock(&bookLock);
    return best;
}

void OrderBook::printOrderBook() {
    pthread_mutex_lock(&bookLock); // lock for thread safety

    std::cout << "\n===== ORDER BOOK (" << symbol << ") =====\n";

    // --- BUY SIDE ---
    std::cout << "--- BUY SIDE ---\n";
    double price = buyTree->getHighestKey(); // start from highest price
    double lowest = buyTree->getLowestKey();

    while (price != -1 && price >= lowest) {
        OrderQueue* q = buyTree->search(price);
        if (q) {
            q->printQueue(); // prints all orders at this price
        }
        double nextPrice = buyTree->nextKey(price); // get next lower price
        if (nextPrice == price || nextPrice == -1) break; 
        price = nextPrice;
    }

    // --- SELL SIDE ---
    std::cout << "--- SELL SIDE ---\n";
    price = sellTree->getLowestKey(); // start from lowest price
    double highest = sellTree->getHighestKey();

    while (price != -1 && price <= highest) {
        OrderQueue* q = sellTree->search(price);
        if (q) {
            q->printQueue(); // prints all orders at this price
        }
        double nextPrice = sellTree->nextKey(price); // get next higher price
        if (nextPrice == price || nextPrice == -1) break;
        price = nextPrice;
    }

    pthread_mutex_unlock(&bookLock);
}


string OrderBook:: getSymbol() const
{
    return symbol;
} 
