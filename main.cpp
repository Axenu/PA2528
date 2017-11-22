#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <chrono>

#include "AllocatorBase.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.h"
#include "StackAllocator.h"
#include "DefaultAllocator.h"
#include "PoolTester.h"
#include "StackTester.h"
#include "StompAllocator.h"
#include "StompScenarios.h"

#if !defined(__WIN32) && !defined(WIN32) && !defined(_WIN32)
//mac
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif


// #define ALLOC(t, args...) currentGlobalAllocator->alloc<t>(__FILE__, __LINE__, args)
// #define ALLOC_ARR(t, size, args...) currentGlobalAllocator->alloc_arr<t>(__FILE__, __LINE__, size, args)
// #define DEALLOC(p) currentGlobalAllocator->dealloc(__FILE__, __LINE__, p)

AllocatorBase *currentGlobalAllocator = nullptr;

struct A
{
  A(int size)
  {
      // std::cout << "ctor A" << std::endl;
      a = (char *)currentGlobalAllocator->alloc_arr<char>(size);
      // a = ALLOC_ARR(char, size);
      // a[0] = 1;
      // a[1] = 10;
      for (int i = 0; i < size; i++) {
          a[i] = 44;
      }
  }
  ~A()
  {
    // std::cout << "dtor A" << std::endl;
    currentGlobalAllocator->dealloc(a);
    // DEALLOC(a);
  }

  char *a;
};

struct B {
    int i;
    B() {
        // std::cout << "ctor B" << std::endl;
        i = 33;
    }
};

struct C {
	float f;
	double d;
	int i;
	char c;

	C() {
		f = 2.0f;
		d = 4.0;
		i = 6;
		c = 8;
	}
};

void poolScenario(size_t numObjects = 1000000) {
	std::cout << "Testing int:\n";
	PoolTester::scenario1<int>(numObjects, 1);
	PoolTester::scenario1<int>(numObjects, 4);
	PoolTester::scenario1<int>(numObjects, 8);
	PoolTester::scenario1<int>(numObjects, 16);

	PoolTester::scenario2<int>(numObjects, 1);
	PoolTester::scenario2<int>(numObjects, 4);
	PoolTester::scenario2<int>(numObjects, 8);
	PoolTester::scenario2<int>(numObjects, 16);

	std::cout << "\n\nTesting struct C:\n";
	PoolTester::scenario1<C>(numObjects, 1);
	PoolTester::scenario1<C>(numObjects, 4);
	PoolTester::scenario1<C>(numObjects, 8);
	PoolTester::scenario1<C>(numObjects, 16);
	PoolTester::scenario1<C>(numObjects, 32);

	PoolTester::scenario2<C>(numObjects, 1);
	PoolTester::scenario2<C>(numObjects, 4);
	PoolTester::scenario2<C>(numObjects, 8);
	PoolTester::scenario2<C>(numObjects, 16);
	PoolTester::scenario2<C>(numObjects, 32);

	std::getchar();
}

void stackScenario()
{
	std::cout << "Stack test:\n";
}

long buddyScenario() {
    const int count = 100;
    char *arr[count];
    int sizes[count];
    //create a set of sizes with sizes from 1 to 512 bytes.
    for (int i = 0; i < count; i++) {
        sizes[i] = (1 << ((i%10))) * 100;
//       std::cout << "size: " << sizes[i] << std::endl;
    }
    long diff;
    //start timer
    #if defined(__WIN32) || defined(WIN32)  || defined(_WIN32)
    	// Start timer
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    #else
    uint64_t start, stop;
    start = mach_absolute_time();
    #endif
    // BuddyAllocator *all = (BuddyAllocator *) currentGlobalAllocator;
    for (int i = 0; i < 1; i++) {
        //all->printMemory(8);
        //allocate the memory
        for (int j = 0; j < count; j++) {
            arr[j] = currentGlobalAllocator->alloc_arr<char>(sizes[j]);
        }
        //use the memory.
        for (int j = 0; j < count; j++) {
            // std::cout << "location: " << arr[j] <<std::endl;
            memset(arr[j], j, sizes[j]);
        }
        //read the memory.
        for (int j = 0; j < count; j++) {
            char *a = arr[j];
             for (int k = 0; k < sizes[j]; k++) {
                 if ((int)a[0] != j) {
                     std::cout << "error, data not persistent: " << a[0] << " and: " << j << std::endl;
                     break;
                 }
             }
        }
        //deallocate the memory
        for (int j = 0; j < count; j++) {
            currentGlobalAllocator->dealloc(arr[j]);
        }
        // BuddyAllocator *all = (BuddyAllocator *) currentGlobalAllocator;
//        all->printMemory(8);
    }
#if defined(__WIN32) || defined(WIN32) || defined(_WIN32)
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  	diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    #else
    stop = mach_absolute_time();
    diff = stop - start;
    #endif

    return diff;
}


void runStompScenarios()
{
    #ifndef ENABLE_STOMP
    std::cout << "Cannot run stomp scenarios. Enable the StompAllocator first by defining ENABLE_STOMP." << std::endl;
    return;
    #endif // ENABLE_STOMP


    std::string checkStr;
    #if ENABLE_STOMP == true
    checkStr = "overrun";
    {
        std::cout << "Running StompAllocator overrun tests..." << std::endl;
        DefaultAllocator allocator;
        stompAccessFreedFailScenario(&allocator);
        stompPassScenario(&allocator);
        stompOverrunFailScenario(&allocator);
    }
    #elif ENABLE_STOMP == false
    checkStr = "underrun";
    {
        std::cout << "Running StompAllocator underrun tests..." << std::endl;
        DefaultAllocator allocator;
        stompAccessFreedFailScenario(&allocator);
        stompPassScenario(&allocator);
        stompUnderrunFailScenario(&allocator);
    }
    #else
    #error Invalid ENABLE_STOMP value
    #endif // ENABLE_STOMP

    {
        std::cout << "Running StompAllocator-BuddyAllocator " << checkStr << " test..." << std::endl;
        BuddyAllocator allocator(StompAllocator::getPageSize() << 12);
        currentGlobalAllocator = &allocator;
        buddyScenario();
    }

    std::cout << "Running StompAllocator-PoolAllocator " << checkStr << " test..." << std::endl;
    poolScenario(10000);
}

long clockFunction(void (*func) ()) {
    //start timer
    //struct timeval stop, start;
    //gettimeofday(&start, NULL);
    //func();
    ////end timer and return
    //gettimeofday(&stop, NULL);
    //long diff = (stop.tv_usec - start.tv_usec) + 1000000 * (stop.tv_sec - start.tv_sec);
    //return diff;
	return 1;
}

int main()
{

    DefaultAllocator dAllocator = DefaultAllocator();


     BuddyAllocator *buddy = new BuddyAllocator(4096 << 12);
//        BuddyAllocator *buddy = new BuddyAllocator(2048);
    currentGlobalAllocator = buddy;
    printf("Buddy allocator took %lu microseconds.\n", buddyScenario());
     currentGlobalAllocator = &dAllocator;
     printf("Buddy scenario with default allocator took %lu microseconds.\n", buddyScenario());

  //   StackAllocator *stack = new StackAllocator(1024, 0);
	 //currentGlobalAllocator = stack;

	 //size_t stackSize = stack->getSizeOfMemory();
	 //std::cout << "Stack size: " << stackSize <<std::endl;

    delete buddy;
	//delete stack;

	//poolScenario();
	//stackScenario();

    return 0;
}
