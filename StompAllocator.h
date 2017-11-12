#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"

#include <vector>

class StompAllocator : public AllocatorBase {
public:
    StompAllocator(bool checkOverrun);
    ~StompAllocator();

    static size_t getPageSize();

private:
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);


private:
    using BlockSize = size_t;
    struct Block {
        BlockSize size;
        char* ptr;
    };
    static const size_t M_PAGE_SIZE;
    const bool M_CHECK_OVERRUN;
    std::vector<Block> mFreeBlocks;
};

#endif
