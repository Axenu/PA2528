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

size_t StompAllocator::getPageSize() {
    return M_PAGE_SIZE;
}

void* StompAllocator::alloc_internal(size_t size) {
    if(M_CHECK_OVERRUN) {
        size += sizeof(BlockSize);
    }

    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    BlockSize blockSize = size + M_PAGE_SIZE + distanceToPageBoundary;
    BlockSize dataSize = blockSize;
    char* block = nullptr;
    for(auto it = mFreeBlocks.begin(); it != mFreeBlocks.end(); it++) {
        if(dataSize <= it->size) {
            block = (char*)it->ptr;
            blockSize = it->size;
            mFreeBlocks.erase(it);
            break;
        }
    }
    if(!block) {
        block = (char*)aligned_malloc(dataSize, M_PAGE_SIZE);
    }

    char* ptr;
    char* protectPage;
    DWORD prevProtect;
    protect(block, dataSize, PAGE_READWRITE, &prevProtect);
    if(M_CHECK_OVERRUN) {
        ptr = block + distanceToPageBoundary;
        *((BlockSize*)ptr) = blockSize;
        ptr += sizeof(BlockSize);
        protectPage = block + dataSize - M_PAGE_SIZE;
    }
    else {
        ptr = block + M_PAGE_SIZE;
        *((BlockSize*)block) = blockSize;
        protectPage = block;
    }
    protect(protectPage, M_PAGE_SIZE, PAGE_NOACCESS, &prevProtect);
    return ptr;
}

void StompAllocator::dealloc_internal(void *p) {
    char* block;
    BlockSize* blockSize;
    if(M_CHECK_OVERRUN) {
        char* ptr = (char*)p - sizeof(BlockSize);
        block = ptr - size_t(ptr) % M_PAGE_SIZE;
        blockSize = (BlockSize*)ptr;
    }
    else {
        block = (char*)p - M_PAGE_SIZE;
        blockSize = (BlockSize*)block;
    }
    DWORD prevProtect;
    protect(blockSize, sizeof(BlockSize), PAGE_READONLY, &prevProtect);

    Block b;
    b.ptr = block;
    b.size = *blockSize;

    protect(b.ptr, b.size, PAGE_NOACCESS, &prevProtect);
    mFreeBlocks.push_back(b);
}
