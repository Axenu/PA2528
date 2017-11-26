#ifndef __STACK_TESTER__
#define __STACK_TESTER__

#include <string>
#include <iostream>
#include <chrono>

#include "StackAllocator.h"

class StackTester
{
public:
	template<typename T>
	static void timeTest(size_t stackSize, size_t numObjects); // tests the stack allocator against the OS allocator

	template<typename T>
	static void timeTestAllocDeallocIntervals(size_t stackSize, size_t objPerInterval, size_t intervals);

	static void overflowTest(); // tries to cause an overflow
};

template<typename T>
inline void StackTester::timeTest(size_t stackSize, size_t numObjects) // allocates all the objects before deallocating
{
	//void** defaultAlloc = new void*[numObjects];
	std::vector<T>* testVec = new std::vector<T>(stackSize);

	int i;

	std::cout << "Testing stack allocation vs OS with stack of size " << stackSize << " and allocation of " << numObjects << " objects." << std::endl;
	
	// OS
	// start timer
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	// run OS test
	for (i = 0; i < numObjects; ++i) // correct implementation for testing vs stack?
	{
		testVec->push_back(sizeof(T));
		//defaultAlloc[i] = malloc(sizeof(T));
	}

	for (i = 0; i < numObjects; ++i)
	{
		testVec->pop_back();
		//free(defaultAlloc[i]);
	}

	//end timer
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	double deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "OS time: " << deltaTime << "ms" << std::endl;
	
	
	// StackAlloc
	StackAllocator* stack = new StackAllocator(stackSize);

	// start timer
	startTime = std::chrono::high_resolution_clock::now();

	// run StackAlloc test
	for (i = 0; i < numObjects; ++i)
	{
		stack->alloc<T>(sizeof(T));
	}
	
	for (i = 0; i < numObjects; ++i)
	{
		stack->dealloc<T>(nullptr);
	}

	// end timer
	endTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "StackAllocator time: " << deltaTime << "ms" << std::endl;

	//delete[] defaultAlloc;
	delete testVec;
	delete stack;
}

template<typename T>
inline void StackTester::timeTestAllocDeallocIntervals(size_t stackSize, size_t objPerInterval, size_t intervals) // allocs and deallocs in interval
{
	//void** defaultAlloc = new void*[objPerInterval];
	std::vector<T>* testVec = new std::vector<T>(stackSize);
	int i, j;

	std::cout << "Testing stack allocation vs OS with stack of size " << stackSize << " over "<< intervals <<" allocations of " << objPerInterval << " objects." << std::endl;

	// OS
	// start timer
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	// run OS test
	for (i = 0; i < intervals; ++i)
	{
		for (j = 0; j < objPerInterval; ++j) // alloc objects
		{
			testVec->push_back(sizeof(T));
			//defaultAlloc[j] = malloc(sizeof(T));
		}

		for (j = 0; j < objPerInterval; ++j) // dealloc objects
		{
			testVec->pop_back();
			//free(defaultAlloc[j]);
		}
	}

	//end timer
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	double deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "OS time: " << deltaTime << "ms" << std::endl;
	
	// StackAlloc
	StackAllocator* stack = new StackAllocator(stackSize);

	// start timer
	startTime = std::chrono::high_resolution_clock::now();

	// run StackAlloc test
	for (i = 0; i < intervals; ++i)
	{
		for (j = 0; j < objPerInterval; ++j) // alloc objects
		{
			stack->alloc<T>(sizeof(T));
		}

		for (j = 0; j < objPerInterval; ++j) // dealloc objects
		{
			stack->dealloc<T>(nullptr);
		}
	}

	// end timer
	endTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "StackAllocator time: " << deltaTime << "ms" << std::endl;

	delete testVec;
	//delete[] defaultAlloc;
	delete stack;
}

inline void StackTester::overflowTest()
{
	StackAllocator* stack = new StackAllocator(64, 0);

	// alloc code

	delete stack;
}

#endif