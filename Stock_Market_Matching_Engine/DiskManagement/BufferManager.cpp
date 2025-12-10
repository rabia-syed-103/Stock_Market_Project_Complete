#include "BufferManager.h"

BufferManager::BufferManager(const string& diskFile, int cacheSize) {
    pageManager = new PageManager(diskFile);
    cache = new LRUCache(cacheSize);
    
    cout << "🚀 BufferManager initialized" << endl;
    cout << "   Disk file: " << diskFile << endl;
    cout << "   Cache size: " << cacheSize << " pages" << endl;
}

BufferManager::~BufferManager() {
    // Flush all dirty pages before destruction
    flushAll();
    
    delete cache;
    delete pageManager;
    
    cout << "💾 BufferManager destroyed (all changes saved)" << endl;
}

char* BufferManager::getPage(int pageID) {
    // Step 1: Check cache first
    if (cache->has(pageID)) {
        // Cache hit! 🎯
        // cout << "✅ Cache HIT for page " << pageID << endl;
        return cache->get(pageID);
    }
    
    // Step 2: Cache miss - load from disk
    // cout << "⚠️ Cache MISS for page " << pageID << " - loading from disk..." << endl;
    
    char* pageData = pageManager->readPage(pageID);
    if (!pageData) {
        cout << "❌ Failed to read page " << pageID << " from disk!" << endl;
        return nullptr;
    }
    
    // Step 3: Put in cache for future use
    cache->put(pageID, pageData, false);  // Not dirty (just read)
    
    // Step 4: Free the disk-read buffer (cache now owns a copy)
    delete[] pageData;
    
    // Step 5: Return pointer from cache
    return cache->get(pageID);
}

void BufferManager::updatePage(int pageID, const char* data) {
    // Ensure page exists in cache
    if (!cache->has(pageID)) {
        // Not in cache - need to load it first (or create new)
        if (pageManager->pageExists(pageID)) {
            // Load existing page
            char* existingPage = pageManager->readPage(pageID);
            cache->put(pageID, existingPage, false);
            delete[] existingPage;
        } else {
            // Brand new page
            char* emptyPage = new char[PAGE_SIZE];
            memset(emptyPage, 0, PAGE_SIZE);
            cache->put(pageID, emptyPage, false);
            delete[] emptyPage;
        }
    }
    
    // Now update in cache and mark dirty
    cache->put(pageID, data, true);  // Mark as dirty
    
    // cout << "✏️ Updated page " << pageID << " (marked dirty)" << endl;
}

int BufferManager::allocateNewPage() {
    int newPageID = pageManager->allocatePage();
    
    // Create empty page in cache
    char* emptyPage = new char[PAGE_SIZE];
    memset(emptyPage, 0, PAGE_SIZE);
    cache->put(newPageID, emptyPage, true);  // Mark dirty (needs write)
    delete[] emptyPage;
    
    cout << "🆕 Allocated new page: " << newPageID << endl;
    return newPageID;
}

void BufferManager::flushAll() {
    cout << "💾 Flushing all dirty pages to disk..." << endl;
    
    int dirtyCount = 0;
    int* dirtyPageIDs = cache->getAllDirtyPageIDs(&dirtyCount);
    
    if (dirtyCount == 0) {
        cout << "   No dirty pages to flush." << endl;
        return;
    }
    
    cout << "   Found " << dirtyCount << " dirty pages" << endl;
    
    for (int i = 0; i < dirtyCount; i++) {
        int pageID = dirtyPageIDs[i];
        char* pageData = cache->get(pageID);
        
        if (pageData) {
            pageManager->writePage(pageID, pageData);
            // cout << "   Flushed page " << pageID << endl;
        }
    }
    
    delete[] dirtyPageIDs;
    
    cout << "✅ Flushed " << dirtyCount << " pages to disk" << endl;
}

bool BufferManager::pageExists(int pageID) {
    // Check cache first
    if (cache->has(pageID)) {
        return true;
    }
    
    // Check disk
    return pageManager->pageExists(pageID);
}

void BufferManager::printStats() {
    cout << "\n📊 BufferManager Stats:" << endl;
    cout << "   Total pages on disk: " << pageManager->getTotalPages() << endl;
    cache->printStats();
}

void BufferManager::clearAll() {
    cache->clear();
    pageManager->clearAllPages();
    cout << "🗑️ All data cleared!" << endl;
}