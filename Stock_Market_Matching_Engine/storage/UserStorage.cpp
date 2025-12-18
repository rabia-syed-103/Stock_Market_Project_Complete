#include "UserStorage.h"
#include <fstream>
#include <iostream>

UserStorage::UserStorage() : storage("data/users.dat") {
    const size_t recSize = sizeof(UserRecord);
    ifstream f("data/users.dat", ios::binary | ios::ate);
    if (!f) return;
    
    size_t fileSize = (size_t)f.tellg();
    f.close();
    
    // Load all existing users into memory
    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        UserRecord rec;
        storage.read(rawOff, &rec, recSize);
        
        User u = User::fromRecord(rec);
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1; // shift to avoid 0
        
        usersMap[storedOff] = u;
        userIDToOffsetMap[u.getUserID()] = storedOff;
    }
    
    cout << "Loaded " << usersMap.size() << " users from storage.\n";
}

UserStorage::~UserStorage() {
    // Nothing to do - StorageManager handles cleanup
}

// Persist a new user
DiskOffset UserStorage::persist(const User& user) {
    UserRecord rec = user.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(UserRecord));
    
    DiskOffset storedOff = rawOff + 1;
    usersMap[storedOff] = user;
    userIDToOffsetMap[user.getUserID()] = storedOff;
    
    return storedOff;
}

// Load user from offset
User UserStorage::load(DiskOffset offset) {
    if (offset == 0) return User(); // invalid
    
    DiskOffset rawOff = offset - 1;
    UserRecord rec;
    storage.read(rawOff, &rec, sizeof(UserRecord));
    
    return User::fromRecord(rec);
}

// Save user back to disk (update)
void UserStorage::save(const User& user, DiskOffset offset) {
    if (offset == 0) return;
    
    DiskOffset rawOff = offset - 1;
    UserRecord rec = user.toRecord();
    storage.write(rawOff, &rec, sizeof(UserRecord));
    
    usersMap[offset] = user;
}

// Get offset for a userID
DiskOffset UserStorage::getOffsetForUser(const string& userID) {
    auto it = userIDToOffsetMap.find(userID);
    if (it == userIDToOffsetMap.end()) return 0;
    return it->second;
}

// Get user by ID (from memory map)
User* UserStorage::getUserByID(const string& userID) {
    DiskOffset offset = getOffsetForUser(userID);
    if (offset == 0) return nullptr;
    
    return &usersMap[offset];
}

// Load all users
vector<User> UserStorage::loadAllUsers() {
    vector<User> result;
    for (auto& [offset, user] : usersMap) {
        result.push_back(user);
    }
    return result;
}

// Update existing user (finds offset automatically)
void UserStorage::updateUser(const User& user) {
    DiskOffset offset = getOffsetForUser(user.getUserID());
    if (offset != 0) {
        save(user, offset);
    } else {
        persist(user); // New user
    }
}