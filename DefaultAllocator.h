#ifndef __DEFAULT_ALLOCATOR__
#define __DEFAULT_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include "AllocatorBase.h"

class DefaultAllocator : public AllocatorBase {
public:
    DefaultAllocator();
    virtual ~DefaultAllocator();

private:
    void *alloc_internal(size_t size);
    void dealloc_internal(void *p);
};

#endif
