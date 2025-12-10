#include "LRUCache.h"

LRUCache::LRUCache(int cap) : capacity(cap), currentTime(0) {
    entries = new CacheEntry[capacity];
    cout << "🗄️ LRU Cache initialized (capacity: " << capacity << " pages)" << endl;
}

LRUCache::~LRUCache() {
    delete[] entries;
    cout << "🗑️ LRU Cache destroyed" << endl;
}

int LRUCache::findEntry(int pageID) {
    for (int i = 0; i < capacity; i++) {
        if (entries[i].isValid && entries[i].pageID == pageID) {
            return i;
        }
    }
    return -1;  // Not found
}

int LRUCache::findLRUEntry() {
    int lruIndex = -1;
    int minTime = currentTime + 1;
    
    // First, try to find an invalid (empty) slot
    for (int i = 0; i < capacity; i++) {
        if (!entries[i].isValid) {
            return i;
        }
    }
    
    // No empty slots, find LRU
    for (int i = 0; i < capacity; i++) {
        if (entries[i].lastUsed < minTime) {
            minTime = entries[i].lastUsed;
            lruIndex = i;
        }
    }
    
    return lruIndex;
}

bool LRUCache::has(int pageID) {
    return findEntry(pageID) != -1;
}

char* LRUCache::get(int pageID) {
    int index = findEntry(pageID);
    
    if (index == -1) {
        return nullptr;  // Cache miss
    }
    
    // Update LRU time
    entries[index].lastUsed = currentTime++;
    
    // Return pointer to cached data
    return entries[index].data;
}

void LRUCache::put(int pageID, const char* data, bool dirty) {
    // Check if already in cache
    int index = findEntry(pageID);
    
    if (index != -1) {
        // Update existing entry
        memcpy(entries[index].data, data, PAGE_SIZE);
        entries[index].isDirty = dirty;
        entries[index].lastUsed = currentTime++;
        return;
    }
    
    // Find slot to insert (may evict LRU)
    index = findLRUEntry();
    
    // If evicting dirty page, we'd need to write it back
    // (In production, you'd call pageManager->writePage here)
    if (entries[index].isValid && entries[index].isDirty) {
        cout << "⚠️ Evicting dirty page " << entries[index].pageID 
             << " (should write back!)" << endl;
        // TODO: Write back to disk via callback
    }
    
    // Clean up old data if exists
    if (entries[index].data) {
        delete[] entries[index].data;
    }
    
    // Insert new page
    entries[index].pageID = pageID;
    entries[index].data = new char[PAGE_SIZE];
    memcpy(entries[index].data, data, PAGE_SIZE);
    entries[index].isDirty = dirty;
    entries[index].lastUsed = currentTime++;
    entries[index].isValid = true;
}

void LRUCache::markDirty(int pageID) {
    int index = findEntry(pageID);
    if (index != -1) {
        entries[index].isDirty = true;
    }
}

void LRUCache::getDirtyPages(int* pageIDs, char** dataBuffers, int* count) {
    *count = 0;
    
    for (int i = 0; i < capacity; i++) {
        if (entries[i].isValid && entries[i].isDirty) {
            pageIDs[*count] = entries[i].pageID;
            dataBuffers[*count] = entries[i].data;
            (*count)++;
        }
    }
}

int* LRUCache::getAllDirtyPageIDs(int* count) {
    // First, count dirty pages
    *count = 0;
    for (int i = 0; i < capacity; i++) {
        if (entries[i].isValid && entries[i].isDirty) {
            (*count)++;
        }
    }
    
    if (*count == 0) {
        return nullptr;
    }
    
    // Allocate array for pageIDs
    int* dirtyPageIDs = new int[*count];
    int idx = 0;
    
    for (int i = 0; i < capacity; i++) {
        if (entries[i].isValid && entries[i].isDirty) {
            dirtyPageIDs[idx++] = entries[i].pageID;
        }
    }
    
    return dirtyPageIDs;
}

void LRUCache::clear() {
    for (int i = 0; i < capacity; i++) {
        if (entries[i].data) {
            delete[] entries[i].data;
            entries[i].data = nullptr;
        }
        entries[i].isValid = false;
        entries[i].isDirty = false;
        entries[i].pageID = -1;
    }
    currentTime = 0;
    cout << "🗑️ Cache cleared" << endl;
}

void LRUCache::printStats() {
    int validCount = 0;
    int dirtyCount = 0;
    
    for (int i = 0; i < capacity; i++) {
        if (entries[i].isValid) {
            validCount++;
            if (entries[i].isDirty) {
                dirtyCount++;
            }
        }
    }
    
    cout << "📊 Cache Stats:" << endl;
    cout << "   Valid pages: " << validCount << "/" << capacity << endl;
    cout << "   Dirty pages: " << dirtyCount << endl;
}