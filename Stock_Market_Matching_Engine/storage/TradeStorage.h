#ifndef TRADESTORAGE_H
#define TRADESTORAGE_H

#include "../core/Trade.h"
#include "StorageManager.h"
#include "DiskTypes.h"
#include <unordered_map>
#include <vector>
#include <mutex>

using namespace std;

class TradeStorage {
private:
    StorageManager storage;
    
    // CHANGED: Only keep index, not full trades
    unordered_map<int, DiskOffset> tradeIDToOffsetMap;  // tradeID -> offset (INDEX ONLY)
    
    // REMOVED: tradesMap - data lives on disk
    
    mutable mutex indexMutex;  // NEW: Thread safety

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
    
    // NEW: Query methods for disk-first design
    Trade loadTrade(int tradeID);
    vector<Trade> loadTradesForUser(const string& userID);
    vector<Trade> loadTradesForSymbol(const string& symbol);
    
    vector<Trade> loadAllTrades();
    int getTradeCount();
    
private:
    // NEW: Index management
    void loadIndex();
    void saveIndex();
    void rebuildIndex();
};

#endif