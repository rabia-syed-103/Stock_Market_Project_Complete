#ifndef USERSTORAGE_H
#define USERSTORAGE_H

#include "../core/User.h"
#include "StorageManager.h"
#include "DiskTypes.h"
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

class UserStorage {
private:
    StorageManager storage;
    
    // Maps to track offsets and IDs
    unordered_map<string, DiskOffset> userIDToOffsetMap;  // username -> offset
    unordered_map<DiskOffset, User> usersMap;             // offset -> User object
    
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
    User* getUserByID(const string& userID);
    vector<User> loadAllUsers();
    
    // Quick update operations
    void updateUser(const User& user);
};

#endif