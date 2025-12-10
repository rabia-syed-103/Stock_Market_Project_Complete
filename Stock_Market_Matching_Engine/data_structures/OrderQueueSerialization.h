#ifndef ORDERQUEUE_SERIALIZER_H
#define ORDERQUEUE_SERIALIZER_H

#include "OrderQueue.h"
#include "../DiskManagement/BufferManager.h"
#include <cstring>

/**
 * OrderQueue Serialization Format:
 * 
 * [queueSize: int]
 * [For each order:]
 *   [orderPageID: int]  ← Reference to where Order is stored
 * 
 * Note: We don't store full Order objects in the queue page.
 * We store pageIDs that reference where Orders are stored.
 */

class OrderQueueSerializer {
public:

    static void serialize(char* pageBuffer, OrderQueue* queue, 
                         int* orderPageIDs, int count) {
        memset(pageBuffer, 0, PAGE_SIZE);
        int offset = 0;
        
        // Write queue size
        memcpy(pageBuffer + offset, &count, sizeof(int));
        offset += sizeof(int);
        
        // Write order pageIDs
        for (int i = 0; i < count; i++) {
            memcpy(pageBuffer + offset, &orderPageIDs[i], sizeof(int));
            offset += sizeof(int);
        }
    }
    

    static int* deserialize(const char* pageBuffer, int* countOut) {
        int offset = 0;
        
        // Read queue size
        int count;
        memcpy(&count, pageBuffer + offset, sizeof(int));
        offset += sizeof(int);
        *countOut = count;
        
        if (count == 0) return nullptr;
        
        // Read order pageIDs
        int* orderPageIDs = new int[count];
        for (int i = 0; i < count; i++) {
            memcpy(&orderPageIDs[i], pageBuffer + offset, sizeof(int));
            offset += sizeof(int);
        }
        
        return orderPageIDs;
    }
    

    static void saveQueueToDisk(BufferManager* buffer, int queuePageID, 
                                OrderQueue* queue, int* orderPageIDs, int count) {
        char* pageBuffer = new char[PAGE_SIZE];
        serialize(pageBuffer, queue, orderPageIDs, count);
        buffer->updatePage(queuePageID, pageBuffer);
        delete[] pageBuffer;
    }

    static int* loadQueueFromDisk(BufferManager* buffer, int queuePageID, 
                                   int* countOut) {
        char* pageData = buffer->getPage(queuePageID);
        if (!pageData) {
            *countOut = 0;
            return nullptr;
        }
        
        return deserialize(pageData, countOut);
    }
};

#endif // ORDERQUEUE_SERIALIZER_H

