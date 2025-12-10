#include "PageManager.h"

PageManager::PageManager(const string& filename) : diskFile(filename), totalPages(0) {
    // Check if file exists
    ifstream checkFile(diskFile, ios::binary);
    if (!checkFile.good()) {
        // File doesn't exist, create it
        ofstream createFile(diskFile, ios::binary);
        createFile.close();
        cout << "📄 Created new disk file: " << diskFile << endl;
    } else {
        // File exists, calculate total pages
        checkFile.seekg(0, ios::end);
        long fileSize = checkFile.tellg();
        totalPages = fileSize / PAGE_SIZE;
        checkFile.close();
        cout << "📂 Opened existing disk file: " << diskFile 
             << " (" << totalPages << " pages)" << endl;
    }
}

PageManager::~PageManager() {
    cout << "💾 PageManager closed. Total pages: " << totalPages << endl;
}

char* PageManager::readPage(int pageID) {
    if (pageID < 0 || pageID >= totalPages) {
        cout << "❌ Invalid pageID: " << pageID << " (total: " << totalPages << ")" << endl;
        return nullptr;
    }
    
    // Allocate 4KB buffer
    char* buffer = new char[PAGE_SIZE];
    memset(buffer, 0, PAGE_SIZE);  // Initialize with zeros
    
    // Open file and seek to page location
    ifstream file(diskFile, ios::binary);
    if (!file.is_open()) {
        cout << "❌ Failed to open file for reading!" << endl;
        delete[] buffer;
        return nullptr;
    }
    
    // Jump to page position (pageID * 4096)
    file.seekg(pageID * PAGE_SIZE, ios::beg);
    
    // Read 4KB
    file.read(buffer, PAGE_SIZE);
    
    if (!file.good() && !file.eof()) {
        cout << "❌ Error reading page " << pageID << endl;
        delete[] buffer;
        file.close();
        return nullptr;
    }
    
    file.close();
    
    // Debug output (optional, remove in production)
    // cout << "✅ Read page " << pageID << " (4KB)" << endl;
    
    return buffer;
}

void PageManager::writePage(int pageID, const char* data) {
    if (pageID < 0) {
        cout << "❌ Invalid pageID: " << pageID << endl;
        return;
    }
    
    // Ensure file has capacity for this page
    ensureCapacity(pageID);
    
    // Open file for writing at specific position
    fstream file(diskFile, ios::binary | ios::in | ios::out);
    if (!file.is_open()) {
        cout << "❌ Failed to open file for writing!" << endl;
        return;
    }
    
    // Jump to page position
    file.seekp(pageID * PAGE_SIZE, ios::beg);
    
    // Write 4KB
    file.write(data, PAGE_SIZE);
    
    if (!file.good()) {
        cout << "❌ Error writing page " << pageID << endl;
    }
    
    file.close();
    
    // Debug output (optional)
    // cout << "✅ Wrote page " << pageID << " (4KB)" << endl;
}

int PageManager::allocatePage() {
    int newPageID = totalPages;
    totalPages++;
    
    // Write empty page to extend file
    char* emptyPage = new char[PAGE_SIZE];
    memset(emptyPage, 0, PAGE_SIZE);
    
    writePage(newPageID, emptyPage);
    
    delete[] emptyPage;
    
    cout << "🆕 Allocated new page: " << newPageID << endl;
    return newPageID;
}

void PageManager::ensureCapacity(int pageID) {
    // If pageID is beyond current file size, extend file
    while (totalPages <= pageID) {
        allocatePage();
    }
}

int PageManager::getTotalPages() const {
    return totalPages;
}

bool PageManager::pageExists(int pageID) {
    return pageID >= 0 && pageID < totalPages;
}

void PageManager::clearAllPages() {
    // Delete and recreate file
    remove(diskFile.c_str());
    
    ofstream createFile(diskFile, ios::binary);
    createFile.close();
    
    totalPages = 0;
    cout << "🗑️ All pages cleared!" << endl;
}