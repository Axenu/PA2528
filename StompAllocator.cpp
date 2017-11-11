#include "StompAllocator.h"

#include <iostream>
#include <cstdlib>

#include <windows.h>

StompAllocator::StompAllocator() {
    std::cout << "ctor StompAllocator" << std::endl;

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << sysInfo.dwPageSize << std::endl;
    for(size_t i = 0; i < 10; i++)
    {
        char* a = new char[sysInfo.dwPageSize - 8];
        std::cout << ((size_t)a) % sysInfo.dwPageSize << std::endl;
    }

}
StompAllocator::~StompAllocator() {
    std::cout << "dtor StompAllocator" << std::endl;
}

void* StompAllocator::alloc_internal(size_t size) {
    return malloc(size);
}

void StompAllocator::dealloc_internal(void *p) {
    free(p);
}
