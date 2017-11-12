#include "StompAllocator.h"

#include <iostream>
#include <cstdlib>

#ifdef __WIN32
#include <malloc.h>
#include <windows.h>
const size_t StompAllocator::M_PAGE_SIZE = []() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}();
#define aligned_malloc(size, alignment) _aligned_malloc(size, alignment)
#define aligned_free(memblock) _aligned_free(memblock)
#define protect(block, size, access, previousAccess) VirtualProtect(block, size, access, previousAccess)
#else
#error Unsupported platform.
#endif

StompAllocator::StompAllocator(bool checkOverrun)
: M_CHECK_OVERRUN(checkOverrun) {
}

StompAllocator::~StompAllocator() {
}

void* StompAllocator::alloc_internal(size_t size) {
    if(M_CHECK_OVERRUN) {
        size += sizeof(BlockSize);
    }

    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    BlockSize blockSize = size + M_PAGE_SIZE + distanceToPageBoundary;
    char* block = (char*)aligned_malloc(blockSize, M_PAGE_SIZE);

    DWORD prevProtect;
    if(M_CHECK_OVERRUN) {
        protect(block + blockSize - M_PAGE_SIZE, M_PAGE_SIZE, PAGE_NOACCESS, &prevProtect);
        char* ptr = block + distanceToPageBoundary;
        *((BlockSize*)ptr) = blockSize;
        return ptr + sizeof(BlockSize);
    }
    else {
        protect(block, M_PAGE_SIZE, PAGE_NOACCESS, &prevProtect);
        return block + M_PAGE_SIZE;
    }
}

void StompAllocator::dealloc_internal(void *p) {
    DWORD prevProtect;
    char* block;
    if(M_CHECK_OVERRUN) {
        char* ptr = (char*)p - sizeof(BlockSize);
        block = ptr - size_t(ptr) % M_PAGE_SIZE;
        BlockSize blockSize = *((BlockSize*)ptr);
        protect(block + blockSize - M_PAGE_SIZE, M_PAGE_SIZE, PAGE_READWRITE, &prevProtect);
    }
    else {
        block = (char*)p - M_PAGE_SIZE;
        protect(block, M_PAGE_SIZE, PAGE_READWRITE, &prevProtect);
    }
    aligned_free(block);
}
