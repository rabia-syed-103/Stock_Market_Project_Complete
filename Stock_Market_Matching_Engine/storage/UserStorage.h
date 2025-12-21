#ifndef USERSTORAGE_H
#define USERSTORAGE_H

#include "../core/User.h"
#include "StorageManager.h"
#include "DiskTypes.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

using namespace std;

class UserStorage {
private:
    StorageManager storage;
    
    // CHANGED: Only keep index in memory, not full user objects
    unordered_map<string, DiskOffset> userIDToOffsetMap;  // username -> offset (INDEX ONLY)
    
    // REMOVED: usersMap - we don't keep full users in memory anymore
    // Data lives on disk, we only cache via the MatchingEngine's LRU cache
    
    mutable mutex indexMutex;  // NEW: Thread safety for index

public:
    UserStorage();
    UserStorage(const UserStorage&) = delete;
    UserStorage& operator=(const UserStorage&) = delete;
    ~UserStorage();
    
    // Core operations
    DiskOffset persist(const User& user);
    User load(DiskOffset offset);
    void save(const User& user, DiskOffset offset);
    
    // Lookup operations
    DiskOffset getOffsetForUser(const string& userID);
    
    // NEW: Direct load by userID (uses index)
    User loadUser(const string& userID);
    
    // NEW: Check if user exists without loading
    bool userExists(const string& userID);
    
    vector<User> loadAllUsers();
    
    // Quick update operations
    void updateUser(const User& user);
    
private:
    // NEW: Index management
    void loadIndex();
    void saveIndex();
    void rebuildIndex();
};

#endif