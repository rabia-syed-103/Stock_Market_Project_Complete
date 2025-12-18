#include "TradeStorage.h"
#include <fstream>
#include <iostream>

TradeStorage::TradeStorage() : storage("data/trades.dat") {
    const size_t recSize = sizeof(TradeRecord);
    ifstream f("data/trades.dat", ios::binary | ios::ate);
    if (!f) return;
    
    size_t fileSize = (size_t)f.tellg();
    f.close();
    
    // Load all existing trades into memory
    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        TradeRecord rec;
        storage.read(rawOff, &rec, recSize);
        
        Trade t = Trade::fromRecord(rec);
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1; // shift to avoid 0
        
        tradesMap[storedOff] = t;
        tradeIDToOffsetMap[t.tradeID] = storedOff;
    }
    
    cout << "Loaded " << tradesMap.size() << " trades from storage.\n";
}

TradeStorage::~TradeStorage() {
    // Nothing to do - StorageManager handles cleanup
}

// Persist a new trade (append-only)
DiskOffset TradeStorage::persist(const Trade& trade) {
    TradeRecord rec = trade.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(TradeRecord));
    
    DiskOffset storedOff = rawOff + 1;
    tradesMap[storedOff] = trade;
    tradeIDToOffsetMap[trade.tradeID] = storedOff;
    
    return storedOff;
}

// Load trade from offset
Trade TradeStorage::load(DiskOffset offset) {
    if (offset == 0) return Trade(); // invalid
    
    DiskOffset rawOff = offset - 1;
    TradeRecord rec;
    storage.read(rawOff, &rec, sizeof(TradeRecord));
    
    return Trade::fromRecord(rec);
}

// Get offset for a tradeID
DiskOffset TradeStorage::getOffsetForTrade(int tradeID) {
    auto it = tradeIDToOffsetMap.find(tradeID);
    if (it == tradeIDToOffsetMap.end()) return 0;
    return it->second;
}

// Load all trades
vector<Trade> TradeStorage::loadAllTrades() {
    vector<Trade> result;
    for (auto& [offset, trade] : tradesMap) {
        result.push_back(trade);
    }
    return result;
}

// Get total trade count
int TradeStorage::getTradeCount() {
    return tradesMap.size();
}