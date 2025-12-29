#include "TradeStorage.h"
#include <fstream>
#include <iostream>

TradeStorage::TradeStorage() : storage("data/trades.dat") {
    // CHANGED: Only load index
    loadIndex();
    cout << "Loaded trade index: " << tradeIDToOffsetMap.size() << " trades.\n";
}

TradeStorage::~TradeStorage() {
    // NEW: Save index on shutdown
    saveIndex();
}

DiskOffset TradeStorage::persist(const Trade& trade) {
    lock_guard<mutex> lock(indexMutex);
    
    TradeRecord rec = trade.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(TradeRecord));
    DiskOffset storedOff = rawOff + 1;
    
    // CHANGED: Only update index
    tradeIDToOffsetMap[trade.tradeID] = storedOff;
    
    return storedOff;
}

Trade TradeStorage::load(DiskOffset offset) {
    if (offset == 0) return Trade();
    
    DiskOffset rawOff = offset - 1;
    TradeRecord rec;
    storage.read(rawOff, &rec, sizeof(TradeRecord));
    return Trade::fromRecord(rec);
}

DiskOffset TradeStorage::getOffsetForTrade(int tradeID) {
    lock_guard<mutex> lock(indexMutex);
    
    auto it = tradeIDToOffsetMap.find(tradeID);
    if (it == tradeIDToOffsetMap.end()) return 0;
    return it->second;
}

// NEW: Load single trade by ID
Trade TradeStorage::loadTrade(int tradeID) {
    DiskOffset offset = getOffsetForTrade(tradeID);
    if (offset == 0) return Trade();
    return load(offset);
}

// NEW: Load trades for a specific user
vector<Trade> TradeStorage::loadTradesForUser(const string& userID) {
    vector<Trade> allTrades = loadAllTrades();
    vector<Trade> userTrades;
    
    for (const Trade& t : allTrades) {
        if (t.buyUserID == userID || t.sellUserID == userID) {
            userTrades.push_back(t);
        }
    }
    
    return userTrades;
}

// NEW: Load trades for a specific symbol
vector<Trade> TradeStorage::loadTradesForSymbol(const string& symbol) {
    vector<Trade> allTrades = loadAllTrades();
    vector<Trade> symbolTrades;
    
    for (const Trade& t : allTrades) {
        if (t.symbol == symbol) {
            symbolTrades.push_back(t);
        }
    }
    
    return symbolTrades;
}

vector<Trade> TradeStorage::loadAllTrades() {
    lock_guard<mutex> lock(indexMutex);
    
    vector<Trade> result;
    result.reserve(tradeIDToOffsetMap.size());
    
    // CHANGED: Load from disk using index
    for (const auto& [tradeID, offset] : tradeIDToOffsetMap) {
        result.push_back(load(offset));
    }
    
    return result;
}

int TradeStorage::getTradeCount() {
    lock_guard<mutex> lock(indexMutex);
    return tradeIDToOffsetMap.size();
}

void TradeStorage::loadIndex() {
    ifstream indexFile("data/trades.idx", ios::binary);

    if (!indexFile) {
        cout << "Trade index not found, rebuilding from trades.dat...\n";
        rebuildIndex();
        return;
    }

    size_t count = 0;
    indexFile.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (!indexFile || count == 0 || count > 1000000) {
        cout << "Trade index invalid or empty, rebuilding from trades.dat...\n";
        indexFile.close();
        rebuildIndex();
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        int tradeID;
        indexFile.read(reinterpret_cast<char*>(&tradeID), sizeof(tradeID));

        DiskOffset offset;
        indexFile.read(reinterpret_cast<char*>(&offset), sizeof(offset));

        if (!indexFile) {
            cout << "Corrupted trade index entry, rebuilding...\n";
            indexFile.close();
            rebuildIndex();
            return;
        }

        tradeIDToOffsetMap[tradeID] = offset;
    }

    indexFile.close();
    cout << "Loaded trade index: " << tradeIDToOffsetMap.size() << " trades.\n";
}

// NEW: Save index to file
void TradeStorage::saveIndex() {
    ofstream indexFile("data/trades.idx", ios::binary);
    if (!indexFile) {
        cerr << "Failed to save trade index\n";
        return;
    }
    
    lock_guard<mutex> lock(indexMutex);
    
    size_t count = tradeIDToOffsetMap.size();
    indexFile.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    for (const auto& [tradeID, offset] : tradeIDToOffsetMap) {
        indexFile.write(reinterpret_cast<const char*>(&tradeID), sizeof(tradeID));
        indexFile.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }
    
    indexFile.close();
}

void TradeStorage::rebuildIndex() {
    tradeIDToOffsetMap.clear();

    const size_t recSize = sizeof(TradeRecord);

    ifstream f("data/trades.dat", ios::binary | ios::ate);
    if (!f) {
        cout << "trades.dat not found, nothing to rebuild\n";
        saveIndex();
        return;
    }

    size_t fileSize = static_cast<size_t>(f.tellg());
    f.close();

    if (fileSize < recSize) {
        cout << "No trades found in trades.dat\n";
        saveIndex();
        return;
    }

    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        TradeRecord rec;
        storage.read(rawOff, &rec, recSize);  // your StorageManager read
        Trade t = Trade::fromRecord(rec);
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1;
        tradeIDToOffsetMap[t.tradeID] = storedOff;
    }

    cout << "Rebuilt trade index: " << tradeIDToOffsetMap.size() << " trades.\n";
    saveIndex();
}