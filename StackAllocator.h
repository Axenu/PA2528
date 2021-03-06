#ifndef __STACK_ALLOCATOR__
#define __STACK_ALLOCATOR__

#include <assert.h>

#include "AllocatorBase.h"

class StackAllocator : public AllocatorBase{

public:
	StackAllocator(size_t sizeStack, const size_t alignment = 0); // size of the stack, and alignment in the memory
	~StackAllocator();

	void reset(); // resets head to the start of the stack

	void* getHead() { return m_head; };		// returns the current "head" of the stack
	size_t getSizeOfMemory();

private:
	virtual void* alloc_internal(size_t size);
	virtual void dealloc_internal(void* p);

private:
	int m_alignment; // alignment of the memory

	size_t m_sizeStack; // size of the memory allocated for the stack

	size_t m_offset; // memory offset used when making new allocations

	// stack pointers
	void* m_start;			// points at the start of the memory block
	void* m_end;			// points at the upper bound (end) of the memory block
	void* m_head;		// points at the current head of the stack (start + the current offset)
};

#endif
