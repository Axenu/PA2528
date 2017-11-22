#ifndef __BUDDY_ALLOCATOR__
#define __BUDDY_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <stdio.h>

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
    short _num_levels;
    int *_buddyArray;
    int *_splitArray;
    int _allocated_size;

    //inherited methods for allocating
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);

    //allocate a block
    int nearestLevel(int size);
    void *getBlockAtLevel(int level);
    void *split(void *, int level);

    // deallocate a block
    int findLevel(void *p);
    void merge(void *p, short level);

    //some bit array functions
    inline void setBit(int *A, int k) {
        A[k/32] |= (1 << (k%32));
    }
    inline void unsetBit(int *A, int k) {
        A[k/32] &= ~(1 << (k%32));
    }
    inline bool getBit(int *A, int k) {
        return ( (A[k/32] & (1 << (k%32) )) != 0 );
    }


    inline int indexInLevelOf(void *p, int n) {
        return (int)((char *)p - (char *)_origin) / sizeOfLevel(n);
    }
    inline int globalSplitIndex(void *p, int n) {
        int levelSize = sizeOfLevel(n);
        int gi = (int)((char *)p - (char *)_origin) / levelSize;
        gi += (1 << (n)) - 1;
        return gi;
    }
    inline int globalBuddyIndex(void *p, int n) {
        return globalSplitIndex(p, n-1);
    }
    inline int sizeOfLevel(short n) {
        return (int)_totalSize/(1<<n);
    }
    inline void *pointerForIndex(int index, short n) {
        return index * sizeOfLevel(n) + (char *)_origin;
    }

};

#endif
