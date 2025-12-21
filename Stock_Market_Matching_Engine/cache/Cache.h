#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <list>
#include <mutex>
#include <memory>
using namespace std;

template<typename K, typename V>

class LRUCache {
private:
    size_t capacity;
    list<std::pair<K, std::shared_ptr<V>>> lruList;
    unordered_map<K, typename list<pair<K, shared_ptr<V>>>::iterator> cacheMap;
    mutable mutex cacheMutex;
    
    // Statistics
    size_t hits = 0;
    size_t misses = 0;

public:
    explicit LRUCache(size_t cap) : capacity(cap) {}

    // Get item from cache (returns nullptr if not found)
    shared_ptr<V> get(const K& key) {
        lock_guard<std::mutex> lock(cacheMutex);
        
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            misses++;
            return nullptr; // Cache miss
        }
        
        hits++;
        
        // Move to front (most recently used)
        lruList.splice(lruList.begin(), lruList, it->second);
        return it->second->second;
    }

    // Put item in cache
    void put(const K& key, shared_ptr<V> value) {
        lock_guard<std::mutex> lock(cacheMutex);
        
        // If key exists, update and move to front
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            it->second->second = value;
            lruList.splice(lruList.begin(), lruList, it->second);
            return;
        }
        
        // If at capacity, evict least recently used
        if (lruList.size() >= capacity) {
            auto last = lruList.back();
            cacheMap.erase(last.first);
            lruList.pop_back();
        }
        
        // Insert new item at front
        lruList.emplace_front(key, value);
        cacheMap[key] = lruList.begin();
    }

    // Remove item from cache
    void remove(const K& key) {
        lock_guard<mutex> lock(cacheMutex);
        
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            lruList.erase(it->second);
            cacheMap.erase(it);
        }
    }

    // Clear entire cache
    void clear() {
        lock_guard<mutex> lock(cacheMutex);
        lruList.clear();
        cacheMap.clear();
    }

    // Get cache statistics
    double getHitRate() const {
        lock_guard<mutex> lock(cacheMutex);
        size_t total = hits + misses;
        return total > 0 ? (double)hits / total : 0.0;
    }

    size_t size() const {
        lock_guard<mutex> lock(cacheMutex);
        return lruList.size();
    }
};

#endif // CACHE_H