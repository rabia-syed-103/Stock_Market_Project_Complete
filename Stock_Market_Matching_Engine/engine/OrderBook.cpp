#include "OrderBook.h"
#include <iostream>
#include <algorithm>

using namespace std;

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
/*
vector<Trade> OrderBook::addOrder(Order* order) {
    pthread_mutex_lock(&bookLock);
    vector<Trade> trades;
    bool isBuy = order->getSide();
    if (isBuy) {

        Order* bestSell = sellTree->getBestSell();
        
        while (bestSell != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestSell->price <= order->price) {
            if (order->userID == bestSell->userID) {
                double currentPrice = bestSell->price;
                double nextPrice = sellTree->nextKey(currentPrice);
                if (nextPrice != -1 && nextPrice != currentPrice) {
                    bestSell = sellTree->search(nextPrice)->peek();
                    continue;
                } else {
                    break;
                }
            }
            int matchedQty = min(order->remainingQty, bestSell->remainingQty);
            
            Trade trade(
                100 + trades.size(),
                order,
                bestSell,
                matchedQty,
                bestSell->price   
            );
            
            trades.push_back(trade);
            order->fill(matchedQty);
            bestSell->fill(matchedQty);
            if (bestSell->isFilled()) {
                sellTree->search(bestSell->price)->dequeue();
            }

            bestSell = sellTree->getBestSell();
        }

        if (!order->isFilled()) {
            buyTree->insert(order->price, order);
        }
        
    } else {

        Order* bestBuy = buyTree->getBest();
        
        while (bestBuy != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestBuy->price >= order->price) {
            
            if (order->userID == bestBuy->userID) {
                double currentPrice = bestBuy->price;
                double nextPrice = buyTree->prevKey(currentPrice);
                if (nextPrice != -1 && nextPrice != currentPrice) {
                    bestBuy = buyTree->search(nextPrice)->peek();
                    continue;
                } else {
                    break;
                }
            }
            
            int matchedQty = std::min(order->remainingQty, bestBuy->remainingQty);
            
            Trade trade(
                200 + trades.size(),
                bestBuy,
                order,
                matchedQty,
                bestBuy->price  
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
    
    pthread_mutex_unlock(&bookLock);
    return trades;
}
*/
vector<Trade> OrderBook::addOrder(Order* order) {
    pthread_mutex_lock(&bookLock);
    vector<Trade> trades;

    bool isBuy = order->getSide();

    if (isBuy) {
        // BUY order: match with best SELL orders
        Order* bestSell = sellTree->getBestSell();

        while (bestSell != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestSell->price <= order->price) {

            // Skip self-trade
            if (order->userID == bestSell->userID) {
                double nextPrice = sellTree->nextKey(bestSell->price);
                bestSell = (nextPrice != -1) ? sellTree->search(nextPrice)->peek() : nullptr;
                if (!bestSell) break;
                continue;
            }

            int matchedQty = std::min(order->getRemainingQuantity(), bestSell->getRemainingQuantity());

            Trade trade(
                100 + trades.size(),
                order,
                bestSell,
                matchedQty,
                bestSell->price
            );

            trades.push_back(trade);

            // Update quantities
            order->reduceRemainingQty(matchedQty);
            bestSell->reduceRemainingQty(matchedQty);

            // Update statuses
            order->status = (order->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";
            bestSell->status = (bestSell->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";

            // Only dequeue if fully filled
            if (bestSell->getRemainingQuantity() == 0) {
                sellTree->search(bestSell->price)->dequeue();
            }

            // Peek next best sell
            bestSell = sellTree->getBestSell();
        }

        // Add unfilled portion to BUY book
        if (order->getRemainingQuantity() > 0) {
            buyTree->insert(order->price, order);
        }

    } else {
        // SELL order: match with best BUY orders
        Order* bestBuy = buyTree->getBest();

        while (bestBuy != nullptr &&
               order->getRemainingQuantity() > 0 &&
               bestBuy->price >= order->price) {

            // Skip self-trade
            if (order->userID == bestBuy->userID) {
                double prevPrice = buyTree->prevKey(bestBuy->price);
                bestBuy = (prevPrice != -1) ? buyTree->search(prevPrice)->peek() : nullptr;
                if (!bestBuy) break;
                continue;
            }

            int matchedQty = std::min(order->getRemainingQuantity(), bestBuy->getRemainingQuantity());

            Trade trade(
                200 + trades.size(),
                bestBuy,
                order,
                matchedQty,
                bestBuy->price
            );

            trades.push_back(trade);

            // Update quantities
            order->reduceRemainingQty(matchedQty);
            bestBuy->reduceRemainingQty(matchedQty);

            // Update statuses
            order->status = (order->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";
            bestBuy->status = (bestBuy->getRemainingQuantity() == 0) ? "FILLED" : "PARTIAL_FILL";

            // Only dequeue if fully filled
            if (bestBuy->getRemainingQuantity() == 0) {
                buyTree->search(bestBuy->price)->dequeue();
            }

            // Peek next best buy
            bestBuy = buyTree->getBest();
        }

        // Add unfilled portion to SELL book
        if (order->getRemainingQuantity() > 0) {
            sellTree->insert(order->price, order);
        }
    }

    pthread_mutex_unlock(&bookLock);
    return trades;
}

void OrderBook::cancelOrder(int orderID) {
    pthread_mutex_lock(&bookLock);
    
    bool found = false;
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
    
    pthread_mutex_unlock(&bookLock);
}

Order* OrderBook::getBestBid() {
    pthread_mutex_lock(&bookLock);
    Order* best = buyTree->getBest();
    pthread_mutex_unlock(&bookLock);
    return best;
}

Order* OrderBook::getBestAsk() {
    pthread_mutex_lock(&bookLock);
    Order* best = sellTree->getBestSell();  
    pthread_mutex_unlock(&bookLock);
    return best;
}

void OrderBook::printOrderBook() {
    pthread_mutex_lock(&bookLock); 

    cout << "\n ORDER BOOK (" << symbol << ") \n";


    cout << " BUY SIDE \n";
    // BUY SIDE - traverse from highest to lowest
    double price = buyTree->getHighestKey();
    double lowest = buyTree->getLowestKey();

    while (price != -1 && price >= lowest) {
        OrderQueue* q = buyTree->search(price);
        if (q && q->getSize() > 0) {
            q->printQueue();
        }
        double nextPrice = buyTree->prevKey(price);  
        if (nextPrice == price || nextPrice == -1) break;
        price = nextPrice;
    }

    //  SELL SIDE 
    cout << " SELL SIDE \n";
    price = sellTree->getLowestKey(); // start from lowest price
    double highest = sellTree->getHighestKey();

    while (price != -1 && price <= highest) {
        OrderQueue* q = sellTree->search(price);
        if (q) {
            q->printQueue(); 
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