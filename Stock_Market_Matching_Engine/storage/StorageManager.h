#pragma once
#include <cstdint>
#include <string>
#include <fstream>
#include <mutex>

using DiskOffset = uint64_t;

class StorageManager {
private:
    std::fstream file;
    std::mutex ioMutex;

public:
    StorageManager(const std::string& filename);
    ~StorageManager();
    StorageManager(const StorageManager&) = delete;
    StorageManager& operator=(const StorageManager&) = delete;

    StorageManager(StorageManager&&) = delete;
    StorageManager& operator=(StorageManager&&) = delete;

    DiskOffset append(const void* data, size_t size);
    void read(DiskOffset offset, void* buffer, size_t size);
    void write(DiskOffset offset, const void* data, size_t size);

    size_t getFileSize();
};
