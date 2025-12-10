#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include "PageManager.h"
#include "LRUCache.h"
#include <iostream>

using namespace std;

class BufferManager {
private:
    PageManager* pageManager;
    LRUCache* cache;
    
public:
    BufferManager(const string& diskFile, int cacheSize = MAX_CACHE_SIZE);
    ~BufferManager();
    

    char* getPage(int pageID);
 
    void updatePage(int pageID, const char* data);
   
    int allocateNewPage();
    

    void flushAll();

    bool pageExists(int pageID);

    void printStats();

    void clearAll();
};

#endif // BUFFERMANAGER_H