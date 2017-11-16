#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

#include "AllocatorBase.h"
#include "PoolAllocator.h"
#include "BuddyAllocator.h"
#include "DefaultAllocator.h"


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

void poolScenario() {
    for(size_t i = 0; i < 1; i++) {
        A *a = currentGlobalAllocator->alloc<A>(10);
        A *b = currentGlobalAllocator->alloc<A>(12);
        // A *a = ALLOC(A, 10);
        // A *b = ALLOC(A, 20);
        // A *a = new(memory) A(12);
        // DEALLOC(a);
        // DEALLOC(b);
        currentGlobalAllocator->dealloc(a);
        currentGlobalAllocator->dealloc(b);
    }
}

long buddyScenario() {
    int count = 100;
    char *arr[count];
    int sizes[count];
    for (int i = 0; i < count; i++) {
        sizes[i] = 1 << ((i%10));
//        std::cout << "size: " << sizes[i] << std::endl;
    }
    //start timer
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < count; j++) {
            arr[j] = currentGlobalAllocator->alloc_arr<char>(sizes[j]);
//            arr[j] = currentGlobalAllocator->alloc_arr<char>(32768);
            //use the memory.
        }
        for (int j = 0; j < count; j++) {
            //use the memory.
            memset(arr[j], 255, sizes[j]);
        }
        for (int j = count-1; j >= 0; j--) {
            currentGlobalAllocator->dealloc(arr[j]);
        }

    }
    //end timer and return
    gettimeofday(&stop, NULL);
    long diff = (stop.tv_usec - start.tv_usec) + 1000000 * (stop.tv_sec - start.tv_sec);
    return diff;
}

long clockFunction(void (*func) ()) {
    //start timer
    struct timeval stop, start;
    gettimeofday(&start, NULL);
    func();
    //end timer and return
    gettimeofday(&stop, NULL);
    long diff = (stop.tv_usec - start.tv_usec) + 1000000 * (stop.tv_sec - start.tv_sec);
    return diff;
}

int main()
{

    DefaultAllocator dAllocator = DefaultAllocator();
    // PoolAllocator *pool = new PoolAllocator(sizeof(int), 4, 4);
    // currentGlobalAllocator = pool;

    // poolScenario();
    // clockFunction(poolScenario);

    BuddyAllocator *buddy = new BuddyAllocator(1048576);
    currentGlobalAllocator = buddy;
    printf("Buddy allocator took %lu microseconds.\n", buddyScenario());
    currentGlobalAllocator = &dAllocator;
    printf("Buddy scenario with default allocator took %lu microseconds.\n", buddyScenario());

    // StackAllocator *stack = new StackAllocator(10,1024, 4);
    // currentGlobalAllocator = stack;
    // poolScenario();

    delete buddy;
    // delete pool;
    return 0;
}
