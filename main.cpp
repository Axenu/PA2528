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

	std::cout << "\n\nTesting pool size:\n";
	PoolTester::scenario1<int>(10000, 1);
	PoolTester::scenario1<int>(100000, 1);
	PoolTester::scenario1<int>(1000000, 1);
	PoolTester::scenario1<int>(10000000, 1);

	std::cout << "\n\nTesting random:\n";
	PoolTester::testRandom();

	std::getchar();
}

void stackScenario(size_t numObjects = 1000000)
{
	std::cout << "Stack allocation tests (int):" << std::endl;
	StackTester::timeTest<int>(1024, numObjects);
	std::cout << std::endl;

	std::cout << "Stack allocation tests (char):" << std::endl;
	StackTester::timeTest<char>(1024, numObjects);
	std::cout << std::endl;

	std::cout << "Stack allocation tests (struct):" << std::endl;
	//StackTester::timeTest<C>(1024, numObjects);
	std::cout << std::endl;

	std::cout << std::endl;
	std::cout << "Stack allocation interval tests (int):" << std::endl;
	StackTester::timeTestAllocDeallocIntervals<int>(1024, numObjects, 1);
	StackTester::timeTestAllocDeallocIntervals<int>(1024, numObjects, 4);
	StackTester::timeTestAllocDeallocIntervals<int>(1024, numObjects, 8);
	StackTester::timeTestAllocDeallocIntervals<int>(1024, numObjects, 16);
	StackTester::timeTestAllocDeallocIntervals<int>(1024, numObjects, 32);
	StackTester::timeTestAllocDeallocIntervals<int>(2048, numObjects, 64);

	std::cout << "Stack allocation interval tests (char):" << std::endl;
	StackTester::timeTestAllocDeallocIntervals<char>(1024, numObjects, 1);
	StackTester::timeTestAllocDeallocIntervals<char>(1024, numObjects, 4);
	StackTester::timeTestAllocDeallocIntervals<char>(1024, numObjects, 8);
	StackTester::timeTestAllocDeallocIntervals<char>(1024, numObjects, 16);
	StackTester::timeTestAllocDeallocIntervals<char>(1024, numObjects, 32);
	StackTester::timeTestAllocDeallocIntervals<char>(2048, numObjects, 64);

	std::cout << std::endl;

	std::getchar();
}

long buddyScenario() {
    const int count = 1000;
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
            memset(arr[j], j%256, sizes[j]);
        }
        //read the memory.
        for (int j = 0; j < count; j++) {
            char *a = arr[j];
             for (int k = 0; k < sizes[j]; k++) {
                 if ((char)a[k] != (char)(j%256)) {
                     std::cout << "error, data not persistent: " << (int)(char)a[k] << " and: " << j%256 << std::endl;
                     break;
                 }
             }
        }
        //deallocate the memory
        for (int j = count-1; j >= 0; j--) {
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

long realisticBuddyScenario();

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


    BuddyAllocator *buddy = new BuddyAllocator(1 << 25);
//        BuddyAllocator *buddy = new BuddyAllocator(2048);
    currentGlobalAllocator = buddy;
    printf("Buddy allocator took %lu microseconds.\n", realisticBuddyScenario());
    currentGlobalAllocator = &dAllocator;
    printf("Buddy scenario with default allocator took %lu microseconds.\n", realisticBuddyScenario());

    long sumBuddy = 0;
    long sumDefault = 0;
    for (int i = 0; i < 10; i++) {
        currentGlobalAllocator = buddy;
        sumBuddy += realisticBuddyScenario();
        currentGlobalAllocator = &dAllocator;
        sumDefault += realisticBuddyScenario();
    }
    sumBuddy /= 10;
    sumDefault /= 10;
    double diff = (double)(sumBuddy)/(double)(sumDefault);
    printf("Buddy performed in %lu and default in %lu.\n", sumBuddy, sumDefault);
    printf("Buddy performed in %f of the default time.\n", diff);

    delete buddy;

	//StackAllocator *stack = new StackAllocator(1024);		// non-aligned
	//   //StackAllocator *stack = new StackAllocator(1024, 0);  // custom alignment defined by user
	//currentGlobalAllocator = stack;

	//size_t stackSize = stack->getSizeOfMemory();
	//std::cout << "Stack size: " << stackSize <<std::endl;

	//delete stack;

	//poolScenario();
	//stackScenario(10);

    return 0;
}


long realisticBuddyScenario() {

    const int initialCount = 100;
    int initialSize = 1 << 16;
    char *initialArr[initialCount];
    int levels = 100;
    const int levelAllocCount = 100;
    int levelSize = 1 << 15;
    char *levelArr[levelAllocCount];
    int frames = 100;
    const int frameAllocCount = 1000;
    int frameSize = 1 << 10;
    char *frameArr[frameAllocCount];


    // Start timer
    long diff = 0;
    #if defined(__WIN32) || defined(WIN32)  || defined(_WIN32)
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    #else
    uint64_t start, stop;
    start = mach_absolute_time();
    #endif

    //allocate inital data
    for (int i = 0; i < initialCount; i++) {
        initialArr[i] = currentGlobalAllocator->alloc_arr<char>(initialSize);
    }
    //6.5 MB
    //use the memory.
    for (int i = 0; i < initialCount; i++) {
        memset(initialArr[i], i%256, initialSize);
    }

    //for every level:
    for (int level = 0; level < levels; level++) {
        //allocate level data
        for (int i = 0; i < levelAllocCount; i++) {
            levelArr[i] = currentGlobalAllocator->alloc_arr<char>(levelSize);
        }
        //9.8MB
        //use the memory.
        for (int i = 0; i < levelAllocCount; i++) {
            memset(levelArr[i], i%256, levelSize);
        }

        //for every frame
        for (int frame = 0; frame < frames; frame++) {
            //allocate frame data
            for (int i = 0; i < frameAllocCount; i++) {
                frameArr[i] = currentGlobalAllocator->alloc_arr<char>(frameSize);
            }
            //total allocations: 100 * 2^16 + 100*2^15 + 1000*2^10
            //use the memory.
            for (int i = 0; i < frameAllocCount; i++) {
                memset(frameArr[i], i%256, frameSize);
            }

            //dealloc frame
            for (int i = 0; i < frameAllocCount; i++) {
                currentGlobalAllocator->dealloc(frameArr[i]);
            }
        }

        //dealloc level
        for (int i = 0; i < levelAllocCount; i++) {
            currentGlobalAllocator->dealloc(levelArr[i]);
        }

    }


    //dealloc inital data
    for (int i = 0; i < initialCount; i++) {
        currentGlobalAllocator->dealloc(initialArr[i]);
    }
    //end timer
    #if defined(__WIN32) || defined(WIN32) || defined(_WIN32)
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  	diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    #else
    stop = mach_absolute_time();
    diff = stop - start;
    #endif
    return diff;
}
