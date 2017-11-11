#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"

class StompAllocator : public AllocatorBase {
public:
    StompAllocator();
    ~StompAllocator();

private:
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);
};

#endif
