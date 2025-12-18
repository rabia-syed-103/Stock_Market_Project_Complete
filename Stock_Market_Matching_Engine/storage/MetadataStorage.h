#ifndef METADATASTORAGE_H
#define METADATASTORAGE_H

#include "StorageManager.h"
#include <string>

using namespace std;

struct Metadata {
    int nextOrderID;
    int nextTradeID;
    int totalUsers;
    int totalOrders;
    int totalTrades;
    time_t lastSaveTime;
    
    // Padding for future expansion
    char reserved[256];
};

class MetadataStorage {
private:
    StorageManager storage;
    
public:
    MetadataStorage();
    MetadataStorage(const MetadataStorage&) = delete;
    MetadataStorage& operator=(const MetadataStorage&) = delete;
    ~MetadataStorage();
    
    // Core operations
    void saveMetadata(const Metadata& meta);
    Metadata loadMetadata();
    bool metadataExists();
};

#endif