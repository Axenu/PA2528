#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"

class StompAllocator : public AllocatorBase {
public:
    StompAllocator(bool checkOverrun);
    ~StompAllocator();

private:
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);


private:
    using BlockSize = size_t;
    static const size_t M_PAGE_SIZE;
    const bool M_CHECK_OVERRUN;
};

#endif
