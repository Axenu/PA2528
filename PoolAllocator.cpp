#include <cassert>

#include "PoolAllocator.h"

PoolAllocator::PoolAllocator(size_t elementSize, size_t numElements, int alignment)
	: numElements{ numElements }, allocatedElements{ 0 } {
	std::cout << "ctor PoolAllocator" << std::endl;

	// Ensure an element is at least the size of a pointer
	this->elementSize = std::max(sizeof(void*), elementSize);

	// Get memory from OS
	uintptr_t rawPointer = reinterpret_cast<uintptr_t>(malloc(elementSize * numElements + alignment));

	// Caculate the misalignment
	uintptr_t misalignedBytes = rawPointer & (alignment - 1);

	// Adjust base pointer
	ptrdiff_t pointerAdjustment = alignment - misalignedBytes;
	basePointer = reinterpret_cast<void*>(rawPointer + pointerAdjustment);

	// In the byte before aligned pointer, store the adjustment value (used for deallocating)
	unsigned char* adjustmentStorage = reinterpret_cast<unsigned char*>(basePointer) - 1;
	*adjustmentStorage = static_cast<unsigned char>(pointerAdjustment);

	// Loop through pool and initialize free list
	for (size_t i = 0; i < numElements - 1; ++i) {
		// Set this pointer to address of next element
		reinterpret_cast<unsigned char**>(basePointer)[i] = &reinterpret_cast<unsigned char*>(basePointer)[(i + 1) * elementSize];
	}

	// Set the last element in the free list to nullptr
	reinterpret_cast<unsigned char**>(basePointer)[(numElements - 1)] = nullptr;

	// Initialize free list pointers
	firstFree = basePointer;
	lastFree = &reinterpret_cast<unsigned char*>(basePointer)[elementSize * (numElements - 1)];
}

PoolAllocator::~PoolAllocator() {
	std::cout << "dtor PoolAllocator" << std::endl;
	assert(allocatedElements == 0);

	// Find the difference between basePointer and the pointer obtained from the OS. This number is stored in the byte before basePointer
	size_t paddingBytes = *(reinterpret_cast<unsigned char*>(basePointer) - 1);

	void* rawPointer = reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(basePointer) - paddingBytes);

	free(rawPointer);
}

void* PoolAllocator::alloc_internal(size_t size) {
	assert(allocatedElements < numElements);

	++allocatedElements;

	void* allocatedAddress = firstFree;

	if (*reinterpret_cast<unsigned char**>(firstFree) != nullptr)
		firstFree = *reinterpret_cast<unsigned char**>(firstFree);
	else {
		firstFree = nullptr;
		lastFree = nullptr;
	}

	return allocatedAddress;
}

void PoolAllocator::dealloc_internal(void *p) {
	uintptr_t deallocatedAddress = reinterpret_cast<uintptr_t>(p);
	uintptr_t base = reinterpret_cast<uintptr_t>(basePointer);

	size_t alignment = *(reinterpret_cast<unsigned char*>(basePointer) - 1);
	assert(deallocatedAddress >= base);
	assert((deallocatedAddress - base) % alignment == 0);
	assert(deallocatedAddress - base < base + numElements * elementSize);

	// Loop through free list to check that the pointer is not already deallocated
	void* iterator = firstFree;
	while (iterator != nullptr) {
		assert(iterator != p);
		iterator = *reinterpret_cast<unsigned char**>(iterator);
	}

	--allocatedElements;

	if (lastFree != nullptr) {
		*reinterpret_cast<unsigned char**>(lastFree) = reinterpret_cast<unsigned char*>(deallocatedAddress);
		lastFree = reinterpret_cast<unsigned char*>(deallocatedAddress);
	}
	else {
		firstFree = reinterpret_cast<void*>(deallocatedAddress);
		lastFree = firstFree;
	}
}