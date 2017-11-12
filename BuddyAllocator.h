#ifndef __BUDDY_ALLOCATOR__
#define __BUDDY_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include <assert.h>

#include "AllocatorBase.h"

#define MIN_SIZE 32

struct BuddyHeader {
    void *next;
    void *prev;
};

class BuddyAllocator : public AllocatorBase {
public:
    BuddyAllocator(size_t blockSize);
    ~BuddyAllocator();

    //debug
    void printMemory(int lines);
private:

    static const int MAX_LEVELS = 32;
    void *_free_lists[MAX_LEVELS];
    size_t _totalSize;
    int _leaf_size;
    void *_origin;
    int _num_levels;
    int *_buddyArray;
    int *_splitArray;

    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);

    void *getBlockAtLevel(int level);
    void *split(void *, int level);

    int nearestLevel(size_t size);
    size_t nearestPowerOfTwo(size_t size);
    int maxBlocksOfLevel(int n);
    int findLevel(void *p);
    void *buddyPointer(void *p, int n);
    void merge(void *p, int level);

    inline void setBit(int *A, int k) {
        A[k/32] |= (1 << (k%32));
        // A[k] = 1;
    }
    inline void unsetBit(int *A, int k) {
        A[k/32] &= ~(1 << (k%32));
        // A[k] = 0;
    }

    inline bool getBit(int *A, int k) {
        return ( (A[k/32] & (1 << (k%32) )) != 0 );
    }

    inline int indexInLevelOf(void *p, int n) {
        return ((char *)p - (char *)_origin) / sizeOfLevel(n);
    }
    inline int globalBuddyIndex(void *p, int n) {
        int levelSize = sizeOfLevel(n-1);
        int index = ((char *)p - (char *)_origin) / (levelSize);
        // std::cout << "p: " << p << " orig: " << _origin << " diff: " << ((char *)p - (char *)_origin) / (levelSize >> 3) << " levelsize: " << levelSize << std::endl;
        index += (1 << (n-1)) - 1;
        return index;
    }
    inline int globalSplitIndex(void *p, int n) {
        int levelSize = sizeOfLevel(n);
        int gi = ((char *)p - (char *)_origin) / levelSize;
        gi += (1 << (n)) - 1;
        return gi;
    }
    inline int sizeOfLevel(int n) {
        return _totalSize/(1<<n);
    }

    inline void *pointerForIndex(int index, int n) {
        return index * sizeOfLevel(n) + (int *)_origin;
    }

};

#endif
