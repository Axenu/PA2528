#ifndef __STACK_ALLOCATOR__
#define __STACK_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include <cassert>

#include "AllocatorBase.h"

class StackAllocator : public AllocatorBase{
public:
	// StackAllocator(size_t size, void* start)
	StackAllocator(void* ptrStart, void* ptrEnd);
	~StackAllocator();

private:
	// alloc_internal (size_t size, size_t allignment, size_t offset) ?
	virtual void* alloc_internal(size_t size, size_t alignment, size_t offset);
	virtual void dealloc_internal(void *p);

	//void free(void* p);
	void align(void* ptr, size_t alignment);


private:
	// stack pointers
	// use template <typename T> (T* m_start ... etc) instead of char* ?
	void* m_ptrStart;
	void* m_ptrEnd;
	void* m_ptrCurrent;
};

#endif