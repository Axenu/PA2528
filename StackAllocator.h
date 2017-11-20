#ifndef __STACK_ALLOCATOR__
#define __STACK_ALLOCATOR__

#include <iostream>
#include <cstdlib>
#include <assert.h>

#include "AllocatorBase.h"

//static const size_t DEFAULT_ALIGN = 4;
//static const size_t ALLOCATION_OFFSET = sizeof(uint32_t);
//static const size_t COUNT_OFFSET = sizeof(uint32_t);

class StackAllocator : public AllocatorBase{
public:
	// StackAllocator(size_t size, void* start)
	StackAllocator(size_t sizeStack, size_t alignment); // pointer to start of the stack, size of the stack, and alignment in the memory
	~StackAllocator();

	void* getCurrentHead() { return m_ptr_stack; };		// returns the current "head" of the stack
	size_t getSizeOfMemory() { return m_sizeStack; };	// used when checking for overflow

private:
	// alloc_internal (size_t size, size_t allignment, size_t offset) ?
	virtual void* alloc_internal(size_t size);
	virtual void dealloc_internal(void *p);

	void free(void* p);		// rolls the stack back to the block before the current one
	void freeBlock();		// clears the entire stack

	void reset();			// resets m_ptr_stack to the initial position of m_start

private:
	size_t m_alignment; // alignment of the memory
	// size_t m_allocElements; // allocated elements
	size_t m_sizeStack; // size of the memory allocated for the stack

	size_t m_offset; // memory offset used when making new allocations

	// stack pointers
	// use template <typename T> (T* m_start ... etc) instead of char* ?
	void* m_start;			// points at the start of the memory block
	void* m_end;			// points at the upper bound (end) of the memory block
	void* m_ptr_stack;		// points at the start + the current offset amount
};

#endif
