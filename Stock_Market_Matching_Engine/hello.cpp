#include <iostream>
#include <vector>
#include "core/Order.h"
#include "core/Trade.h"
#include "engine/OrderBook.h"

using namespace std;

int main() {
    cout << "I am new";
    // --- Create OrderBook for AAPL ---
    OrderBook orderBook("AAPL");

    // --- Create Orders ---
    Order* o1 = new Order(1, "U100", "AAPL", "BUY", 50.0, 100);
    Order* o2 = new Order(2, "U200", "AAPL", "SELL", 50.0, 95);
    Order* o3 = new Order(3, "U300", "AAPL", "SELL", 52.0, 50);
    Order* o4 = new Order(4, "U100", "AAPL", "BUY", 51.0, 70);

    // --- Add Orders and match ---
    cout << "=== Placing Orders and Matching ===" << endl;
    vector<Trade> trades1 = orderBook.addOrder(o1);
    vector<Trade> trades2 = orderBook.addOrder(o2);
    vector<Trade> trades3 = orderBook.addOrder(o3);
    vector<Trade> trades4 = orderBook.addOrder(o4);

    // --- Print Trades ---
    cout << "\n=== Trades Executed ===" << endl;
    for (Trade& t : trades1) cout << t.toString() << endl;
    for (Trade& t : trades2) cout << t.toString() << endl;
    for (Trade& t : trades3) cout << t.toString() << endl;
    for (Trade& t : trades4) cout << t.toString() << endl;

    // --- Print Order Book ---
    cout << "\n=== Current Order Book ===" << endl;
    orderBook.printOrderBook();

    // --- Cancel an order ---
    cout << "\n=== Cancel OrderID 3 ===" << endl;
    orderBook.cancelOrder(3);

    // --- Print Order Book after cancellation ---
    cout << "\n=== Order Book After Cancel ===" << endl;
    orderBook.printOrderBook();

    // --- Check Best Bid / Ask ---
    Order* bestBid = orderBook.getBestBid();
    Order* bestAsk = orderBook.getBestAsk();
    cout << "\nBest Bid: " << (bestBid ? bestBid->toString() : "None") << endl;
    cout << "Best Ask: " << (bestAsk ? bestAsk->toString() : "None") << endl;

    return 0;
}
