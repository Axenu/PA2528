#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <chrono>

#include "AllocatorBase.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.h"
#include "DefaultAllocator.h"
#include "PoolTester.h"

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

void poolScenario() {
	std::cout << "Testing int:\n";
	PoolTester::scenario1<int>(1000000, 1);
	PoolTester::scenario1<int>(1000000, 4);
	PoolTester::scenario1<int>(1000000, 8);
	PoolTester::scenario1<int>(1000000, 16);

	PoolTester::scenario2<int>(1000000, 1);
	PoolTester::scenario2<int>(1000000, 4);
	PoolTester::scenario2<int>(1000000, 8);
	PoolTester::scenario2<int>(1000000, 16);

	std::cout << "\n\nTesting struct C:\n";
	PoolTester::scenario1<C>(1000000, 1);
	PoolTester::scenario1<C>(1000000, 4);
	PoolTester::scenario1<C>(1000000, 8);
	PoolTester::scenario1<C>(1000000, 16);
	PoolTester::scenario1<C>(1000000, 32);
						  
	PoolTester::scenario2<C>(1000000, 1);
	PoolTester::scenario2<C>(1000000, 4);
	PoolTester::scenario2<C>(1000000, 8);
	PoolTester::scenario2<C>(1000000, 16);
	PoolTester::scenario2<C>(1000000, 32);

	std::getchar();
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

    // StackAllocator *stack = new StackAllocator(10,1024, 4);
    // currentGlobalAllocator = stack;

    delete buddy;

	//poolScenario();

    return 0;
}
