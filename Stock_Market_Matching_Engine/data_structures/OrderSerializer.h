#ifndef ORDER_SERIALIZER_H
#define ORDER_SERIALIZER_H

#include "../core/Order.h"
#include <cstring>
#include <ctime>
#define PAGE_SIZE 4096
/**
 * Order Serialization Format (fits in one 4KB page):
 * 
 * [orderID: int]
 * [userIDLen: int]
 * [userID: char[]]
 * [symbolLen: int]
 * [symbol: char[]]
 * [sideLen: int]
 * [side: char[]]
 * [price: double]
 * [quantity: int]
 * [remainingQty: int]
 * [statusLen: int]
 * [status: char[]]
 * [timestamp: time_t]
 */

class OrderSerializer {
public:
    /**
     * Serialize Order to page buffer (4KB)
     */
    static void serialize(char* pageBuffer, Order* order) {
        memset(pageBuffer, 0, PAGE_SIZE);
        int offset = 0;
        
        // OrderID
        memcpy(pageBuffer + offset, &order->orderID, sizeof(int));
        offset += sizeof(int);
        
        // UserID (length-prefixed string)
        int userIDLen = order->userID.length();
        memcpy(pageBuffer + offset, &userIDLen, sizeof(int));
        offset += sizeof(int);
        memcpy(pageBuffer + offset, order->userID.c_str(), userIDLen);
        offset += userIDLen;
        
        // Symbol
        int symbolLen = order->symbol.length();
        memcpy(pageBuffer + offset, &symbolLen, sizeof(int));
        offset += sizeof(int);
        memcpy(pageBuffer + offset, order->symbol.c_str(), symbolLen);
        offset += symbolLen;
        
        // Side
        int sideLen = order->side.length();
        memcpy(pageBuffer + offset, &sideLen, sizeof(int));
        offset += sizeof(int);
        memcpy(pageBuffer + offset, order->side.c_str(), sideLen);
        offset += sideLen;
        
        // Price
        memcpy(pageBuffer + offset, &order->price, sizeof(double));
        offset += sizeof(double);
        
        // Quantity
        memcpy(pageBuffer + offset, &order->quantity, sizeof(int));
        offset += sizeof(int);
        
        // Remaining quantity
        memcpy(pageBuffer + offset, &order->remainingQty, sizeof(int));
        offset += sizeof(int);
        
        // Status
        int statusLen = order->status.length();
        memcpy(pageBuffer + offset, &statusLen, sizeof(int));
        offset += sizeof(int);
        memcpy(pageBuffer + offset, order->status.c_str(), statusLen);
        offset += statusLen;
        
        // Timestamp
        memcpy(pageBuffer + offset, &order->timestamp, sizeof(time_t));
    }
    
    /**
     * Deserialize Order from page buffer
     */
    static Order* deserialize(const char* pageBuffer) {
        int offset = 0;
        
        // OrderID
        int orderID;
        memcpy(&orderID, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // UserID
        int userIDLen;
        memcpy(&userIDLen, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        char userIDBuffer[256];
        memcpy(userIDBuffer, pageBuffer + offset, userIDLen);
        userIDBuffer[userIDLen] = '\0';
        string userID(userIDBuffer);
        offset += userIDLen;
        
        // Symbol
        int symbolLen;
        memcpy(&symbolLen, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        char symbolBuffer[50];
        memcpy(symbolBuffer, pageBuffer + offset, symbolLen);
        symbolBuffer[symbolLen] = '\0';
        string symbol(symbolBuffer);
        offset += symbolLen;
        
        // Side
        int sideLen;
        memcpy(&sideLen, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        char sideBuffer[10];
        memcpy(sideBuffer, pageBuffer + offset, sideLen);
        sideBuffer[sideLen] = '\0';
        string side(sideBuffer);
        offset += sideLen;
        
        // Price
        double price;
        memcpy(&price, pageBuffer + offset, sizeof(double));
        offset += sizeof(double);
        
        // Quantity
        int quantity;
        memcpy(&quantity, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // Remaining quantity
        int remainingQty;
        memcpy(&remainingQty, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        
        // Status
        int statusLen;
        memcpy(&statusLen, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        char statusBuffer[50];
        memcpy(statusBuffer, pageBuffer + offset, statusLen);
        statusBuffer[statusLen] = '\0';
        string status(statusBuffer);
        offset += statusLen;
        
        // Timestamp
        time_t timestamp;
        memcpy(&timestamp, pageBuffer + offset, sizeof(time_t));
        
        // Create Order object
        Order* order = new Order(orderID, userID, symbol, side, price, quantity);
        order->remainingQty = remainingQty;
        order->status = status;
        order->timestamp = timestamp;
        
        return order;
    }
};

#endif // ORDER_SERIALIZER_H