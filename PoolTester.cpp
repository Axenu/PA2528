#include "PoolTester.h"

void PoolTester::testCorrectness(size_t numObjects, int alignment) {
	std::cout << "Testing PoolAllocator correctness.\n";

	PoolAllocator* pool = new PoolAllocator(sizeof(size_t), numObjects, alignment);
	size_t** arr = new size_t*[numObjects];

	for (size_t i = 0; i < numObjects; ++i) {
		arr[i] = pool->alloc<size_t>(i);
	}

	bool noError = true;
	for (size_t i = 0; i < numObjects; ++i) {
		if (*arr[i] != i && noError) {
			std::cout << "Error!\n";
			noError = false;
		}
		pool->dealloc<size_t>(arr[i]);
	}

	if (noError)
		std::cout << "No errors detected.\n";

	delete arr;
}