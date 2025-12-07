#include "core/Order.h"
#include "core/Trade.h"
#include "core/User.h"
#include "iostream"

using namespace std;
int main()
{
    User buyer("U100", 50000);   // buyer has 50k cash
    User seller("U200", 0);       // seller starts with 0 cash

    // seller owns 100 shares
    seller.addStock("AAPL", 100);

    // CREATE BUY ORDER  
    Order buyOrder(
        1,
        "U100",
        "AAPL",
        "BUY",
        50,
        100     // buyer wants 50 shares at 100 price
    );

    // CREATE SELL ORDER  
    Order sellOrder(
        2,
        "U200",
        "AAPL",
        "SELL",
        50,
        95      // seller wants 50 shares at 95 price → matchable
    );

    cout << "=== BEFORE MATCH ===\n";
    cout << buyer.toString() << endl;
    cout << seller.toString() << endl;
    cout << buyOrder.toString() << endl;
    cout << sellOrder.toString() << endl;

    // PRICE AGREED = SELL PRICE
    double tradePrice = sellOrder.price;
    int tradeQty = 50;

    // CREATE TRADE
    Trade trade(
        99,
        &buyOrder,
        &sellOrder,
        tradePrice,
        tradeQty
    );

    // EXECUTE TRADE LOGIC
    buyer.deductCash(tradeQty * tradePrice);
    buyer.addStock("AAPL", tradeQty);

    seller.removeStock("AAPL", tradeQty);
    seller.addCash(tradeQty * tradePrice);

    cout << "\n=== AFTER MATCH ===\n";
    cout << trade.toString() << endl;
    cout << buyer.toString() << endl;
    cout << seller.toString() << endl;

    return 0;
}