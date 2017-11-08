#ifndef __POOL_ALLOCATOR__
#define __POOL_ALLOCATOR__

#include <iostream>
#include <cstdlib>

#include "AllocatorBase.h"

class PoolAllocator : public AllocatorBase {
public:
    PoolAllocator() {
        std::cout << "ctor PoolAllocator" << std::endl;
    }
    ~PoolAllocator() {
        std::cout << "dtor PoolAllocator" << std::endl;
    }
    // template <typename T, typename... Args>
private:
    virtual void* alloc_internal(size_t size) {
        return malloc(size);
    }
    virtual void dealloc_internal(void *p) {
        free(p);
    }
};

#endif
