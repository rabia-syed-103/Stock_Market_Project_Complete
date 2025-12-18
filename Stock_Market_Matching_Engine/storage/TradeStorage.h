#ifndef TRADESTORAGE_H
#define TRADESTORAGE_H

#include "../core/Trade.h"
#include "StorageManager.h"
#include "DiskTypes.h"
#include <unordered_map>
#include <vector>

using namespace std;

class TradeStorage {
private:
    StorageManager storage;
    
    // Maps to track offsets and IDs
    unordered_map<int, DiskOffset> tradeIDToOffsetMap;  // tradeID -> offset
    unordered_map<DiskOffset, Trade> tradesMap;         // offset -> Trade object
    
public:
    TradeStorage();
    TradeStorage(const TradeStorage&) = delete;
    TradeStorage& operator=(const TradeStorage&) = delete;
    ~TradeStorage();
    
    // Core operations
    DiskOffset persist(const Trade& trade);
    Trade load(DiskOffset offset);
    
    // Lookup operations
    DiskOffset getOffsetForTrade(int tradeID);
    vector<Trade> loadAllTrades();
    int getTradeCount();
};

#endif