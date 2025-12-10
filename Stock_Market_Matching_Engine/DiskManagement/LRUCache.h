#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <iostream>
#include <cstring>

#define PAGE_SIZE 4096
#define MAX_CACHE_SIZE 100  // Keep 100 pages in memory (400KB)

using namespace std;

// Simple cache entry
struct CacheEntry {
    int pageID;
    char* data;          // 4KB page data
    bool isDirty;        // Needs to be written back to disk?
    int lastUsed;        // For LRU tracking
    bool isValid;        // Is this slot occupied?
    
    CacheEntry() : pageID(-1), data(nullptr), isDirty(false), 
                   lastUsed(0), isValid(false) {}
    
    ~CacheEntry() {
        if (data) {
            delete[] data;
            data = nullptr;
        }
    }
};

class LRUCache {
private:
    CacheEntry* entries;  
    int capacity;
    int currentTime;      

    int findEntry(int pageID);
    

    int findLRUEntry();
    
public:
    LRUCache(int cap = MAX_CACHE_SIZE);
    ~LRUCache();
    

    bool has(int pageID);
    
    
    char* get(int pageID);
    
    void put(int pageID, const char* data, bool dirty = false);
    
    
    void markDirty(int pageID);
    
    
    void getDirtyPages(int* pageIDs, char** dataBuffers, int* count);

    void clear();
    
    // Stats
    void printStats();
    
    
    int* getAllDirtyPageIDs(int* count);
};

#endif // LRUCACHE_H