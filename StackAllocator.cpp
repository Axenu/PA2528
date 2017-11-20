#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t sizeStack, size_t alignment)
{
	std::cout << "ctor StackAllocator" << std::endl;

	m_offset = 1;

	// store alignment
	m_alignment = alignment;

	// allocate memory from the OS to the stack
	std::cout << "allocating memory of size " << sizeStack * sizeof(size_t) << " for the stack allocator" << std::endl;
	if (m_alignment > 0)
		m_start = _aligned_malloc(sizeStack * sizeof(size_t), alignment);
	else
		m_start = malloc(sizeStack * sizeof(size_t));

	// store a pointer to the end of the memory block
	m_end = static_cast<char*>(m_start) + sizeStack * sizeof(size_t);
	
	// set the current "end" of the used memory as the start pointer
	m_ptr_stack = m_start;
}

StackAllocator::~StackAllocator()
{
	std::cout << "dtor StackAllocator" << std::endl;
	free(m_start); // clear the stack
}

void* StackAllocator::alloc_internal(size_t size)
{
	std::cout << "allocating item of size: " << size << std::endl;
	void* current_pointer;

	current_pointer = m_ptr_stack;

	m_ptr_stack = static_cast<char*>(m_ptr_stack) + m_offset * sizeof(size_t);
	//m_ptr_stack = static_cast<char*>(m_ptr_stack) + size * sizeof(size_t);
	//m_ptr_stack = static_cast<char*>(m_ptr_stack) + OFFSET; ???

	// add check for overflow

	return current_pointer;
}

void StackAllocator::dealloc_internal(void* p) // no in pointer needed?
{
	freeBlock();
}

void StackAllocator::free(void* p)
{
	dealloc(p);
}

void StackAllocator::freeBlock()
{
	dealloc(m_ptr_stack);
	// move the m_ptr_stack back to the previous block
	//m_ptr_stack = m_start + m_offset
	// m_ptr_stack -= SIZE_OF_BLOCK ???
	m_ptr_stack = static_cast<char*>(m_ptr_stack) - m_offset * sizeof(size_t);
}

void StackAllocator::reset()
{
	m_ptr_stack = m_start;
}
