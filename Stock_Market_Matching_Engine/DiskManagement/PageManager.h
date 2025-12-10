#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include <string>
#include <fstream>
#include <cstring>
#include <iostream>

#define PAGE_SIZE 4096  // 4KB per page

using namespace std;

class PageManager {
private:
    string diskFile;
    int totalPages;
    
    
    void ensureCapacity(int pageID);
    
public:
    
    PageManager(const string& filename);
    

    ~PageManager();
    

    char* readPage(int pageID);
    
 
    void writePage(int pageID, const char* data);
    

    int allocatePage();
    
    int getTotalPages() const;
    

    bool pageExists(int pageID);
    

    void clearAllPages();
};

#endif // PAGEMANAGER_H