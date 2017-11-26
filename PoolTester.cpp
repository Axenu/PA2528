#include "PoolTester.h"
#include "DefaultAllocator.h"

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

	delete []arr;
}

void PoolTester::testRandom()
{
	std::cout << "Testing \"random\" allocations and deallocations.\n";

	PoolAllocator* pool = new PoolAllocator(sizeof(int), 1000000);
	DefaultAllocator* def = new DefaultAllocator();

	std::cout << "OS:\t\t\n";
	randomHelper(def);

	std::cout << "Pool Allocator:\t\n";
	randomHelper(pool);

	delete def;
	delete pool;
}

void PoolTester::randomHelper(AllocatorBase* allocator)
{
	size_t counter = 0;
	std::vector<int*> pointers;

	// Start timer
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	// Allocate 200 000 objects and then deallocate 100 000 in arbitrary order
	for (int i = 0; i < 5; ++i)
	{
		for (size_t j = 0; j < 200000; ++j)
		{
			pointers.push_back(allocator->alloc<int>());
		}

		for (size_t j = 0; j < 100000; ++j)
		{
			if (pointers[counter] != nullptr)
			{
				allocator->dealloc(pointers[counter]);
				pointers[counter] = nullptr;
			}
			counter += 307;
			counter %= pointers.size();
		}
	}

	// Dealloc the rest of the elements
	for (size_t i = 0; i < pointers.size(); ++i)
	{
		if (pointers[i] != nullptr)
		{
			allocator->dealloc(pointers[i]);
		}
	}

	// End timer
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << duration << " microseconds\n";
}
