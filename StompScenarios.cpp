#include "StompScenarios.h"
#include "AllocatorBase.h"

void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < 10000; size++) {
        char* a = cga->alloc_arr<char>(size);
        char b = a[size];
        a[size] = b;
        cga->dealloc(a);
    }
}

void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < 10000; size++) {
        char* a = cga->alloc_arr<char>(size);
        char b = a[-1];
        a[-1] = b;
        cga->dealloc(a);
    }
}

void stompPassScenario(AllocatorBase* currentGlobalAllocator) {
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < 10000; size++) {
        char* a = cga->alloc_arr<char>(size);
        char* b = cga->alloc_arr<char>(size);
        memcpy(a, b, size);
        cga->dealloc(a);
    }
}
