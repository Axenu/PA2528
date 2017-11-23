#ifndef __STACK_TESTER__
#define __STACK_TESTER__

#include <string>
#include <iostream>
#include <chrono>

#include "StackAllocator.h"

class StackTester
{
public:
	static void timeTest(size_t stackSize, size_t numObjects); // tests the stack allocator against the OS allocator

	static void overflowTest(); // tries to cause an overflow

	static void underflowTest(); // tries to cause an underflow
};

// template<typename T>
inline void StackTester::timeTest(size_t stackSize, size_t numObjects)
{
	size_t** testArr = new size_t*[numObjects];

	std::cout << "Testing stack allocation vs OS with stack of size " << stackSize << " and allocation of " << numObjects << " objects." << std::endl;
	
	// OS
	// start timer
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	// run OS test
	for (int i = 0; i < numObjects; ++i) // correct implementation for testing vs stack?
	{
		testArr[i] = new size_t;
	}

	delete testArr;

	//end timer
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	double deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "OS time in microsec: " << deltaTime << std::endl;
	
	
	// StackAlloc
	StackAllocator* stack = new StackAllocator(stackSize, 0);

	// start timer
	startTime = std::chrono::high_resolution_clock::now();

	// run StackAlloc test
	for (int i = 0; i < numObjects; ++i)
	{
		stack->alloc<size_t>(i);
	}

	stack->dealloc<size_t>(testArr[0]);

	// end timer
	endTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	std::cout << "StackAllocator time in microsec: " << deltaTime << std::endl;

	delete[] testArr;
	delete stack;
}

inline void StackTester::overflowTest()
{
	StackAllocator* stack = new StackAllocator(64, 0);


	delete stack;
}

inline void StackTester::underflowTest()
{
	StackAllocator* stack = new StackAllocator(0, 0);
	delete stack;
}

#endif