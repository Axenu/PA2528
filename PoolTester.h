#ifndef POOLTESTER_H
#define POOLTESTER_H

#include <string>
#include <iostream>
#include <chrono>

#include "PoolAllocator.h"

class PoolTester {
public:
	template<typename T>
	static void scenario1(size_t numObjects, int alignment);

	template<typename T>
	static void scenario2(size_t numObjects, int alignment);

	static void testCorrectness(size_t numObjects, int alignment);

	static void testRandom();

private:
	static void randomHelper(AllocatorBase* allocator);	// Called from testRandom()
};

template<typename T>
inline void PoolTester::scenario1(size_t numObjects, int alignment) {
	std::cout << "Testing pool allocator vs OS with " << numObjects << " objects and alignment " << alignment << ". Released in arbitrary order\n";

	// Test OS
	T** arr = new T*[numObjects];

	// Start timer
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < numObjects; ++i) {
		arr[i] = new T();
	}

	for (size_t i = 0; i < numObjects; ++i) {
		delete arr[(i * 307) % numObjects];
	}

	// End timer
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "OS:\t\t" << duration << " microseconds\n";


	// Test PoolAllocator
	PoolAllocator* pool = new PoolAllocator(sizeof(T), numObjects, alignment);

	// Start timer
	start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < numObjects; ++i) {
		arr[i] = pool->alloc<T>();
	}

	for (size_t i = 0; i < numObjects; ++i) {
		pool->dealloc<T>(arr[(i * 307) % numObjects]);
	}

	// End timer
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "PoolAllocator:\t" << duration << " microseconds\n";

	delete[] arr;
	delete pool;
}

template<typename T>
inline void PoolTester::scenario2(size_t numObjects, int alignment)
{
	std::cout << "Testing pool allocator vs OS with " << numObjects << " objects and alignment " << alignment << ". Released in creation order.\n";

	// Test OS
	T** arr = new T*[numObjects];

	// Start timer
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < numObjects; ++i) {
		arr[i] = new T();
	}

	for (size_t i = 0; i < numObjects; ++i) {
		delete arr[i];
	}

	// End timer
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "OS:\t\t" << duration << " microseconds\n";


	// Test PoolAllocator
	PoolAllocator* pool = new PoolAllocator(sizeof(T), numObjects, alignment);

	// Start timer
	start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < numObjects; ++i) {
		arr[i] = pool->alloc<T>();
	}

	for (size_t i = 0; i < numObjects; ++i) {
		pool->dealloc<T>(arr[i]);
	}

	// End timer
	end = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << "PoolAllocator:\t" << duration << " microseconds\n";

	delete[] arr;
	delete pool;
}

#endif
