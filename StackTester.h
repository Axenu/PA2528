#ifndef __STACK_TESTER__
#define __STACK_TESTER__

class AllocatorBase;

void bufferOverflowTest(AllocatorBase* currentGlobalAllocator);
void bufferUnderflowTest(AllocatorBase* currentGlobalAllocator);

void runStackScenarios();

#endif