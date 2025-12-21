#include "UserStorage.h"
#include <fstream>
#include <iostream>

UserStorage::UserStorage() : storage("data/users.dat") {
    // CHANGED: Only load the index, not full user objects
    loadIndex();
    cout << "Loaded user index: " << userIDToOffsetMap.size() << " users.\n";
}

UserStorage::~UserStorage() {
    // NEW: Save index on shutdown
    saveIndex();
}

// Persist a new user
DiskOffset UserStorage::persist(const User& user) {
    lock_guard<mutex> lock(indexMutex);
    
    UserRecord rec = user.toRecord();
    DiskOffset rawOff = storage.append(&rec, sizeof(UserRecord));
    DiskOffset storedOff = rawOff + 1;
    
    // CHANGED: Only update index, don't store full user in memory
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
    
    lock_guard<mutex> lock(indexMutex);
    
    DiskOffset rawOff = offset - 1;
    UserRecord rec = user.toRecord();
    storage.write(rawOff, &rec, sizeof(UserRecord));
    
    // REMOVED: Don't update usersMap - it doesn't exist anymore
    // Data is on disk, cache handles memory copies
}

// Get offset for a userID
DiskOffset UserStorage::getOffsetForUser(const string& userID) {
    lock_guard<mutex> lock(indexMutex);
    
    auto it = userIDToOffsetMap.find(userID);
    if (it == userIDToOffsetMap.end()) return 0;
    return it->second;
}

// NEW: Load user directly by ID
User UserStorage::loadUser(const string& userID) {
    DiskOffset offset = getOffsetForUser(userID);
    if (offset == 0) {
        return User("", 0); // User doesn't exist
    }
    return load(offset);
}

// NEW: Check if user exists
bool UserStorage::userExists(const string& userID) {
    lock_guard<mutex> lock(indexMutex);
    return userIDToOffsetMap.find(userID) != userIDToOffsetMap.end();
}

// Load all users (reads from disk)
vector<User> UserStorage::loadAllUsers() {
    lock_guard<mutex> lock(indexMutex);
    
    vector<User> result;
    result.reserve(userIDToOffsetMap.size());
    
    // CHANGED: Load from disk using index
    for (const auto& [userID, offset] : userIDToOffsetMap) {
        result.push_back(load(offset));
    }
    
    return result;
}

// Update existing user
void UserStorage::updateUser(const User& user) {
    DiskOffset offset = getOffsetForUser(user.getUserID());
    if (offset != 0) {
        save(user, offset);
    } else {
        persist(user); // New user
    }
}

void UserStorage::loadIndex() {
    ifstream indexFile("data/users.idx", ios::binary);
    
    if (!indexFile) {
        cout << "User index not found, rebuilding...\n";
        rebuildIndex();
        return;
    }
    
    // ✅ ADD: Check file size
    indexFile.seekg(0, ios::end);
    size_t fileSize = indexFile.tellg();
    indexFile.seekg(0, ios::beg);
    
    if (fileSize < sizeof(size_t)) {
        cout << "User index corrupted, rebuilding...\n";
        indexFile.close();
        rebuildIndex();
        return;
    }
    
    size_t count;
    indexFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    // ✅ ADD: Sanity check
    if (count > 1000000) {
        cout << "User index has invalid count, rebuilding...\n";
        indexFile.close();
        rebuildIndex();
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        size_t idLen;
        indexFile.read(reinterpret_cast<char*>(&idLen), sizeof(idLen));
        
        // ✅ ADD: Validate length
        if (idLen > 100) {
            cout << "Invalid userID length, rebuilding...\n";
            indexFile.close();
            rebuildIndex();
            return;
        }
        
        string userID(idLen, '\0');
        indexFile.read(&userID[0], idLen);
        
        DiskOffset offset;
        indexFile.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        
        userIDToOffsetMap[userID] = offset;
    }
    
    indexFile.close();
    cout << "Loaded user index: " << userIDToOffsetMap.size() << " users.\n";
}

// NEW: Save index to file
void UserStorage::saveIndex() {
    ofstream indexFile("data/users.idx", ios::binary);
    if (!indexFile) {
        cerr << "Failed to save user index\n";
        return;
    }
    
    lock_guard<mutex> lock(indexMutex);
    
    size_t count = userIDToOffsetMap.size();
    indexFile.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    for (const auto& [userID, offset] : userIDToOffsetMap) {
        // Write userID length
        size_t idLen = userID.size();
        indexFile.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
        
        // Write userID
        indexFile.write(userID.c_str(), idLen);
        
        // Write offset
        indexFile.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }
    
    indexFile.close();
}

// NEW: Rebuild index from data file
void UserStorage::rebuildIndex() {
    userIDToOffsetMap.clear();
    
    const size_t recSize = sizeof(UserRecord);
    ifstream f("data/users.dat", ios::binary | ios::ate);
    if (!f) return;
    
    size_t fileSize = (size_t)f.tellg();
    f.close();
    
    // Scan through file and build index
    for (size_t rawOff = 0; rawOff + recSize <= fileSize; rawOff += recSize) {
        UserRecord rec;
        storage.read(rawOff, &rec, recSize);
        
        User u = User::fromRecord(rec);
        DiskOffset storedOff = static_cast<DiskOffset>(rawOff) + 1;
        
        userIDToOffsetMap[u.getUserID()] = storedOff;
    }
    
    cout << "Rebuilt user index: " << userIDToOffsetMap.size() << " users.\n";
    
    // Save the rebuilt index
    saveIndex();
}