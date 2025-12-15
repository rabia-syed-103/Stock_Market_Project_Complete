#include "StorageManager.h"
#include <iostream>
#include <sys/stat.h>

StorageManager::StorageManager(const std::string& filename) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        // create file if not exists
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
}

StorageManager::~StorageManager() {
    file.close();
}

DiskOffset StorageManager::append(const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(ioMutex);

    file.seekp(0, std::ios::end);
    DiskOffset offset = file.tellp();
    file.write(reinterpret_cast<const char*>(data), size);
    file.flush();
    return offset;
}

void StorageManager::read(DiskOffset offset, void* buffer, size_t size) {
    std::lock_guard<std::mutex> lock(ioMutex);

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(buffer), size);
}

void StorageManager::write(DiskOffset offset, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(ioMutex);
    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(data), size);
    file.flush(); // Ensure data is written to disk
}

size_t StorageManager::getFileSize() {
    if (!file.is_open()) return 0;
    
    auto current = file.tellg();
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(current);
    
    return size;
}