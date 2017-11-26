#include "StackTester.h"
#include "StackAllocator.h"
#include "AllocatorBase.h"

void bufferOverflowTest(AllocatorBase* currentGlobalAllocator)
{
	bool error = false;

	std::cout << "Running stack allocator buffer overflow test..." << std::endl;
	
	//test code
	size_t a = 0, b = 0; // init to 0 to stop compiler complaints

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
