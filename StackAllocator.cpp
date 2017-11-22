#include "StackAllocator.h"

StackAllocator::StackAllocator(size_t sizeStack, size_t alignment)
{
	std::cout << "ctor StackAllocator" << std::endl;

	// default offset
	m_offset = 1;

	// store alignment
	m_alignment = alignment;

	// store size
	m_sizeStack = sizeStack;

	// allocate memory from the OS to the stack
	std::cout << "allocating memory of size " << sizeStack * sizeof(size_t) << " for the stack allocator" << std::endl;
	if (m_alignment > 0)
#if defined(__WIN32) || defined(WIN32)  || defined(_WIN32)
		m_start = _aligned_malloc(sizeStack * sizeof(size_t), alignment);
#else
		// m_start = aligned_alloc(alignment, sizeStack * sizeof(size_t));
		posix_memalign(&m_start, alignment, sizeStack * sizeof(size_t));
#endif
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

size_t StackAllocator::getSizeOfMemory()
{
	return m_sizeStack * sizeof(size_t);
}

void* StackAllocator::alloc_internal(size_t size)
{
	std::cout << "allocating item of size: " << size << std::endl;
	void* current_pointer;

	current_pointer = m_ptr_stack;

	m_ptr_stack = static_cast<char*>(m_ptr_stack) + m_offset * sizeof(size_t);
	//m_ptr_stack = static_cast<char*>(m_ptr_stack) + size * sizeof(size_t);
	//m_ptr_stack = static_cast<char*>(m_ptr_stack) + OFFSET;

	// add check for overflow
	if (m_ptr_stack > m_end)
		m_ptr_stack = current_pointer;

	return current_pointer;
}

void StackAllocator::dealloc_internal(void* p) // no in pointer needed?
{
	freeBlock();
}

void StackAllocator::free(void* p) // clears the whole stack
{
	dealloc(p);
}

void StackAllocator::freeBlock() // clears the top-most block of the stack
{
	if (m_ptr_stack != m_start) // make sure the stack pointer is pointing ahead of the start of the stack to prevent underflow errors
	{
		// dealloc memory from the current top-most block
		dealloc(m_ptr_stack);
		
		// move the stack pointer back to the previous block
		m_ptr_stack = static_cast<char*>(m_ptr_stack) - m_offset * sizeof(size_t);
		
		if (m_ptr_stack < m_start) // check for underflow
		{
			std::cout << "No block available to remove from stack, underflow prevented." << std::endl;
			m_ptr_stack = m_start;
		}
			
	}
}

void StackAllocator::reset()
{
	// reset the stack pointer back to the start of the stack
	m_ptr_stack = m_start;
}
