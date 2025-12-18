#include "MetadataStorage.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <ctime>

MetadataStorage::MetadataStorage() : storage("data/metadata.dat") {
    // Constructor is simple - StorageManager handles file creation
}

MetadataStorage::~MetadataStorage() {
    // Nothing to do - StorageManager handles cleanup
}

void MetadataStorage::saveMetadata(const Metadata& meta) {
    // Always write at offset 0 (overwrite)
    storage.write(0, &meta, sizeof(Metadata));
}

Metadata MetadataStorage::loadMetadata() {
    Metadata meta;
    memset(&meta, 0, sizeof(Metadata));
    
    if (!metadataExists()) {
        // Return default values if no metadata exists
        meta.nextOrderID = 1;
        meta.nextTradeID = 1;
        meta.totalUsers = 0;
        meta.totalOrders = 0;
        meta.totalTrades = 0;
        meta.lastSaveTime = time(nullptr);
        return meta;
    }
    
    storage.read(0, &meta, sizeof(Metadata));
    return meta;
}

bool MetadataStorage::metadataExists() {
    return storage.getFileSize() >= sizeof(Metadata);
}