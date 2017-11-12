#include "StackAllocator.h"

/*
Each allocation will generate an overhead of 4 bytes (sizeof(uint32_t))
The overhead is stored in front of each allocation
*/
static const size_t ALLOC_OFFSET_SIZE = sizeof(uint32_t);
static_assert(ALLOC_OFFSET_SIZE == 4, "ALLOC_OFFSET_SIZE is the wrong size.");

StackAllocator::StackAllocator(void* ptrStart, void* ptrEnd)
{	
	std::cout << "ctor StackAllocator" << std::endl;
	
	m_ptrStart = ptrStart;
	m_ptrEnd = ptrEnd;
	m_ptrCurrent = ptrStart;
}

StackAllocator::~StackAllocator()
{
	std::cout << "dtor PoolAllocator" << std::endl;
		
	
}

void* StackAllocator::alloc_internal(size_t size, size_t alignment, size_t offset)
{

	std::cout << "allocate of size: " << size << " | alignment: " << alignment << " | offset: " << offset << std::endl;

	// the alloc offset is stored in front of the allocation
	size += ALLOC_OFFSET_SIZE;
	offset += ALLOC_OFFSET_SIZE;

	const uint32_t allocOffset = (uint32_t)m_ptrCurrent - (uint32_t)m_ptrStart;

	// offset ptr, align, then offset
	align(m_ptrCurrent, alignment); // m_ptrCurrent = align(m_ptrCurrent + offset, alignment) - offset;

	// check if out of memory
	if ((size_t)m_ptrCurrent + size > (size_t)m_ptrEnd)
	{
		// oom
		return nullptr;
	}

	// wip
	return m_ptrCurrent;
}

void StackAllocator::dealloc_internal(void* p)
{
	std::cout << "dealloc StackAllocator" << std::endl;
	free(p);
}

void StackAllocator::align(void* ptr, size_t alignment)
{
	// alignment code
}

//When freeing the allocation grab the 4 bytes in front of the allocation and set the ptr to that.
//void StackAllocator::free(void* ptr)
//{
//	// get alloc offset from the 4 bytes in the front of the allocation
//	
//	m_ptrCurrent = m_ptrStart + 
//}