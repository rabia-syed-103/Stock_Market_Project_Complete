#ifndef SYMBOLSTORAGE_H
#define SYMBOLSTORAGE_H
#include <vector>
#include "StorageManager.h"
#include <algorithm>
#include <string>
#include <cstring>

using namespace std;
class SymbolStorage {
private:
    StorageManager storage;

public:
    SymbolStorage() : storage("data/symbols.dat") {}

void addSymbol(const std::string& symbol) {
    vector<std::string> symbols = loadAllSymbols();
    if (find(symbols.begin(), symbols.end(), symbol) == symbols.end()) {
        storage.append(symbol.c_str(), symbol.size() + 1);
    }
}

vector<string> loadAllSymbols() {
    vector<std::string> result;
    size_t size = storage.getFileSize();
    
    if (size == 0) return result;
    
    char buffer[256];
    size_t offset = 0;
    
    while (offset < size) {
        memset(buffer, 0, sizeof(buffer));  // ⚠️ CRITICAL: Clear buffer
        
        size_t remaining = size - offset;
        if (remaining == 0) break;
        
        storage.read(offset, buffer, min(sizeof(buffer) - 1, remaining));
        
        size_t len = strlen(buffer);
        if (len == 0) break;  // ⚠️ CRITICAL: Prevent infinite loop
        
        result.push_back(std::string(buffer));
        offset += len + 1;
    }
    
    return result;
}
};

#endif
