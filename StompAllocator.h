#ifdef __WIN32
#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"

class StompAllocator : public AllocatorBase {
public:
    StompAllocator(AllocatorBase* allocator, bool doCheckOverrun);
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
    AllocatorBase* mAllocator;
    Block* mFreeBlocks;
    size_t mNumFreeBlocks;
};

#endif
#endif
