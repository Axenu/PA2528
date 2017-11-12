#include <iostream>
#include <cstdlib>
#include <ctime>

#include "AllocatorBase.h"
#include "PoolAllocator.h"


// #define ALLOC(t, args...) currentGlobalAllocator->alloc<t>(__FILE__, __LINE__, args)
// #define ALLOC_ARR(t, size, args...) currentGlobalAllocator->alloc_arr<t>(__FILE__, __LINE__, size, args)
// #define DEALLOC(p) currentGlobalAllocator->dealloc(__FILE__, __LINE__, p)

AllocatorBase *currentGlobalAllocator = nullptr;

struct A
{
  A(int size)
  {
      std::cout << "ctor A" << std::endl;
      a = (char *)currentGlobalAllocator->alloc_arr<char>(size);
      // a = ALLOC_ARR(char, size);
      a[0] = 1;
      a[1] = 10;
  }
  ~A()
  {
    std::cout << "dtor A" << std::endl;
    currentGlobalAllocator->dealloc(a);
    // DEALLOC(a);
  }

  char *a;
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

long clockFunction(void (*func) ()) {
    //start timer
    func();
    //end timer and return
    return 1;
}

int main()
{
    PoolAllocator *pool = new PoolAllocator(sizeof(int), 4, 4);
    currentGlobalAllocator = pool;
    // poolScenario();
    clockFunction(poolScenario);

    // StackAllocator *stack = new StackAllocator(10,1024, 4);
    // currentGlobalAllocator = stack;
    // poolScenario();

    delete pool;
    return 0;
}
