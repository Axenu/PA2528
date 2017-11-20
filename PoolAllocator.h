#ifndef __POOL_ALLOCATOR__
#define __POOL_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cstddef>

#include "AllocatorBase.h"

class PoolAllocator : public AllocatorBase {
public:
	PoolAllocator(size_t elementSize, size_t numElements, int alignment = 1); // Alignment must a be power of 2
	~PoolAllocator();

private:
	virtual void* alloc_internal(size_t size);
	virtual void dealloc_internal(void *p);

	void* basePointer;

	// First and last elements of the free list
	void* firstFree;
	void* lastFree;

	size_t elementSize;

	// Number of currently allocated elements
	size_t allocatedElements;
};

#endif
