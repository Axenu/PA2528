#ifdef __WIN32

#include "StompAllocator.h"

#include <iostream>
#include <cstdlib>
// #error Unsupported platform.

#include <malloc.h>
#include <windows.h>
#include <cassert>

const size_t StompAllocator::M_PAGE_SIZE = []() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}();

StompAllocator::StompAllocator(AllocatorBase* allocator, bool doCheckOverrun)
#ifdef ENABLE_STOMP
: AllocatorBase(AllocatorBase::StompFlag())
#else
: AllocatorBase()
#endif // ENABLE_STOMP
, M_CHECK_OVERRUN(doCheckOverrun)
, mAllocator(allocator)
{
//    mNumFreeBlocks = 0;
//    mFreeBlocks = (Block*)alloc_internal(M_PAGE_SIZE * 100000);
}

StompAllocator::~StompAllocator() {
//    Block* freeBlocks = mFreeBlocks;
//    Block b;
//    mFreeBlocks = &b;
//    mNumFreeBlocks = 0;
//    dealloc_internal((void*)freeBlocks);
}

size_t StompAllocator::getPageSize() {
    return M_PAGE_SIZE;
}

void* StompAllocator::alloc_internal(size_t size) {
//    std::cout << "stomp alloc" << std::endl;
    if(M_CHECK_OVERRUN) {
        size += sizeof(BlockSize) * 2;
    }

    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    BlockSize blockSize = size + M_PAGE_SIZE * 2 + distanceToPageBoundary;
    #ifdef ENABLE_STOMP
    char* block = (char*)mAllocator->alloc_internal(blockSize);
    #else
    char* block = (char*)malloc(blockSize);
    #endif // ENABLE_STOMP
    BlockSize alignmentOffset = (M_PAGE_SIZE - size_t(block) % M_PAGE_SIZE) % M_PAGE_SIZE;


    DWORD prevProtect;
    VirtualProtect(block + alignmentOffset, blockSize - M_PAGE_SIZE, PAGE_READWRITE, &prevProtect);
    char* ptr;
    char* protectPage;
    if(M_CHECK_OVERRUN) {
        ptr = block + alignmentOffset + distanceToPageBoundary;
        protectPage = block + alignmentOffset + blockSize - M_PAGE_SIZE * 2;
    }
    else {
        ptr = block + alignmentOffset + M_PAGE_SIZE - sizeof(BlockSize) * 2;
        protectPage = block + alignmentOffset;
    }
    *((BlockSize*)ptr) = ptr - block;
    ptr += sizeof(BlockSize);
    *((BlockSize*)ptr) = blockSize;
    ptr += sizeof(BlockSize);
    VirtualProtect(protectPage, M_PAGE_SIZE, PAGE_NOACCESS, &prevProtect);
    return ptr;
}

void StompAllocator::dealloc_internal(void *p) {
//    std::cout << "stomp dealloc" << std::endl;


    char* ptr = (char*)p;

    DWORD prevProtect;
    if(!M_CHECK_OVERRUN) {
        assert(size_t(ptr) % M_PAGE_SIZE == 0);
        VirtualProtect(ptr - 1, 1, PAGE_READWRITE, &prevProtect);
    }

    ptr -= sizeof(BlockSize);
    BlockSize blockSize = *(BlockSize*)ptr;
    ptr -= sizeof(BlockSize);
    BlockSize offset = *(BlockSize*)ptr;


    char* block = ptr - offset;
    BlockSize alignmentOffset = (M_PAGE_SIZE - size_t(block) % M_PAGE_SIZE) % M_PAGE_SIZE;
    if(M_CHECK_OVERRUN) {
        VirtualProtect(block + alignmentOffset + blockSize - M_PAGE_SIZE * 2, M_PAGE_SIZE, PAGE_READWRITE, &prevProtect);
    }

    #ifdef ENABLE_STOMP
    mAllocator->dealloc_internal(block);
    #else
    free(block);
    #endif // ENABLE_STOMP

//    VirtualProtect(block + alignmentOffset, blockSize - M_PAGE_SIZE, PAGE_NOACCESS, &prevProtect);






//    char* block;
//    BlockSize* blockSize;
//    if(M_CHECK_OVERRUN) {
//        char* ptr = (char*)p - sizeof(BlockSize);
//        block = ptr - size_t(ptr) % M_PAGE_SIZE;
//        blockSize = (BlockSize*)ptr;
//    }
//    else {
//        block = (char*)p - M_PAGE_SIZE;
//        blockSize = (BlockSize*)block;
//    }
//    DWORD prevProtect;
//    VirtualProtect(blockSize, sizeof(BlockSize), PAGE_READONLY, &prevProtect);
//
//    Block b;
//    b.ptr = block;
//    b.size = *blockSize;
//
//    VirtualProtect(b.ptr, b.size, PAGE_NOACCESS, &prevProtect);
//    mFreeBlocks[mNumFreeBlocks] = b;
//    mNumFreeBlocks++;
}

#endif // __WIN32
