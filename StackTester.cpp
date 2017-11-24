#include "StackTester.h"
#include "StackAllocator.h"
#include "AllocatorBase.h"

void bufferOverflowTest(AllocatorBase* currentGlobalAllocator)
{
	bool error = false;

	std::cout << "Running stack allocator buffer overflow test..." << std::endl;
	
	//test code
	size_t a, b;

	// add code to initialize a & b

	size_t result = a + b;
	if (result < a) // overflow
	{
		error = true;
	}
	//

	if (error)
	{
		std::cout << "Stack overflow test encountered errors!\n";
	}

	if (!error)
		std::cout << "Stack overflow test finished without errors!\n";
}

void bufferUnderflowTest(AllocatorBase* currentGlobalAllocator)
{
	bool error = false;

	std::cout << "Running StackAllocator buffer underflow test..." << std::endl;

	if (currentGlobalAllocator->alloc<size_t>(-1) < 0)
	{
		error = true;
		std::cout << "Stack underflow test encountered errors!\n";
	}

	if(!error)
		std::cout << "Stack underflow test finished without errors!\n";
}

void runStackScenarios()
{
	StackAllocator* stack = new StackAllocator(64, 0);
	bufferOverflowTest(stack);
	bufferUnderflowTest(stack);
}
