#ifdef __WIN32

#include "StompAllocator.h"

#include <iostream>
#include <cstdlib>
// #error Unsupported platform.

#include <malloc.h>
#include <windows.h>
//#define VirtualProtect(...) if(!VirtualProtect(__VA_ARGS__)) {std::cout << "VP fail: " << GetLastError() << " -- (" << __LINE__ << ") " << #__VA_ARGS__ << std::endl;}
#include <cassert>
bool aboooo = false;

const size_t StompAllocator::M_PAGE_SIZE = []() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}();

char* StompAllocator::p166 = 0;

StompAllocator* StompAllocator::mStompAllocators;
size_t StompAllocator::mNumInstances = 0;
StompAllocator::PageHolder StompAllocator::mPageHolder;

bool StompAllocator::isAllocating() {
    assert(mStompAllocators);
    return mStompAllocators->mIsAllocating;
}

void StompAllocator::toggleAllocating(bool flag) {
    mStompAllocators = this;
    mIsAllocating = flag;
}


void StompAllocator::addDirtyGuardPage(void* p) {
    StompAllocator* allocator = mStompAllocators;
    assert(allocator != nullptr);
    assert(allocator->mNumDirtyGuardPages < M_NUM_PAGES);
    allocator->mDirtyGuardPages[allocator->mNumDirtyGuardPages] = p;
    allocator->mNumDirtyGuardPages++;
}
size_t tIdA;
size_t threadId;
namespace STOMP_ALLOCATOR_PRIVATE {
LONG CALLBACK guardPageHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    size_t tIdB = GetCurrentThreadId();
    if(ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        void* ptr = (void*)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
        if(!StompAllocator::isAllocating()) {
//            std::cout << "thread A: " << tIdA << std::endl;
//            std::cout << "thread B: " << tIdB << std::endl;
            DWORD prevProtect;
            VirtualProtect(ptr, 1, PAGE_NOACCESS, &prevProtect);

            std::cout << __FUNCTION__ << " " << size_t(ptr) << std::endl;
//            char b = *(char*)0;
//            char a = *(char*)ptr; // I can't find a better way to actually retrigger the exception
//            return EXCEPTION_CONTINUE_EXECUTION;
        }
//            std::cout << __FUNCTION__ << " " << size_t(ptr) << std::endl;
        if(size_t(ptr) / 4096 == size_t(StompAllocator::p166) / 4096) {
//            std::cout << "p166: " << size_t(ptr) << std::endl;
            aboooo = true;
        }
        assert(GetCurrentThreadId() == threadId);
        StompAllocator::addDirtyGuardPage(ptr);
//        if(StompAllocator::p166 == (char*)1) std::cout << __FUNCTION__ << std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "allocating? " << (int)StompAllocator::isAllocating()<< std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "ptr " << size_t(ptr) << std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "mod " << size_t(ptr) % 4096 << std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "p166 " << size_t(StompAllocator::p166) << std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "same page? "  << int(size_t(StompAllocator::p166) / 4096 == size_t(ptr) / 4096) << std::endl;
//        if(StompAllocator::p166 == (char*)1) std::cout << "d "  << size_t(ptr) - size_t(StompAllocator::p166) << std::endl;

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}
}
using namespace STOMP_ALLOCATOR_PRIVATE;


StompAllocator::StompAllocator(AllocatorBase& allocator, bool doCheckOverrun)
#ifdef ENABLE_STOMP
: AllocatorBase(AllocatorBase::StompFlag())
#else
: AllocatorBase()
#endif // ENABLE_STOMP
, M_CHECK_OVERRUN(doCheckOverrun)
, mAllocator(allocator)
{
    mNumInstances++;
//    static bool doOnce = []() {
////        memset(mStompAllocators, 0, sizeof(mStompAllocators));
//        return true;
//    }();
//    mNumFreeBlocks = 0;
//    mFreeBlocks = (Block*)alloc_internal(M_PAGE_SIZE * 100000);
}

StompAllocator::PageHolder::PageHolder() {
    mStompAllocators = nullptr;
    AddVectoredExceptionHandler(1, guardPageHandler);
    threadId = GetCurrentThreadId();


    int a;
    allocator = new StompAllocator(*(AllocatorBase*)&a, true);
}

StompAllocator::PageHolder::~PageHolder() {
    for(char* p = (char*)minPage; p <= maxPage; p += M_PAGE_SIZE) {
        DWORD prevProtect;
        VirtualProtect(p, 1, PAGE_READWRITE, &prevProtect);
    }
    delete allocator;
}

StompAllocator::~StompAllocator() {
    mNumInstances--;
    if(mNumInstances == 0) {
        mStompAllocators = mPageHolder.allocator;
//        mStompAllocators = nullptr;
//        for(char* p = (char*)mPageHolder.minPage; p <= mPageHolder.maxPage; p += M_PAGE_SIZE) {
//            DWORD prevProtect;
//            bool success = VirtualProtect(p, 1, PAGE_READWRITE, &prevProtect);
////            if(success && prevProtect != PAGE_READWRITE | PAGE_GUARD) {
////                VirtualProtect(p, 1, prevProtect, &prevProtect);
////            }
//        }
    }
//    for(size_t i = 0; i < mNumGuardPages; i++) {
//        DWORD prevProtect;
//        VirtualProtect(mGuardPages[i], 1, PAGE_READWRITE, &prevProtect);
//    }
//    Block* freeBlocks = mFreeBlocks;
//    Block b;
//    mFreeBlocks = &b;
//    mNumFreeBlocks = 0;
//    dealloc_internal((void*)freeBlocks);
}

size_t StompAllocator::getPageSize() {
    return M_PAGE_SIZE;
}

char* allocatedPtrs[1000000];
size_t numAllocatedPtrs = 0;
void* StompAllocator::alloc_internal(size_t size) {
//    std::cout << "stomp alloc " << size << std::endl;
    if(M_CHECK_OVERRUN) {
        size += sizeof(BlockSize) * 2;
    }
    tIdA = GetCurrentThreadId();

    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    BlockSize blockSize = size + M_PAGE_SIZE * 2 + distanceToPageBoundary;
    toggleAllocating(true);
    char* block = (char*)mAllocator.alloc_internal(blockSize);
    toggleAllocating(false);
    allocatedPtrs[numAllocatedPtrs] = block;
    numAllocatedPtrs++;
    size_t blockBeginPage = size_t(block) / M_PAGE_SIZE;
    size_t blockEndPage = size_t(block + blockSize) / M_PAGE_SIZE;
//    std::cout << "Reserving pages " << blockBeginPage + 1 << "-" << blockEndPage - 1 << std::endl;
    DWORD prevProtect;
    static bool reservedPages[1000000];
    static bool isDone = false;
    if(!isDone) {
        memset(reservedPages, 0, sizeof(reservedPages));
    }
//    for(size_t i = blockBeginPage + 1; i < blockEndPage; i++) {
//        assert(!reservedPages[i]);
//        reservedPages[i] = true;
//    }
    for(size_t i = 0; i < mNumDirtyGuardPages; i++) {
//        size_t dirtyPage = size_t(mDirtyGuardPages[i]) / M_PAGE_SIZE;
//        if(dirtyPage < blockBeginPage || dirtyPage >= blockEndPage) {
            VirtualProtect(mDirtyGuardPages[i], 1, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
//        }
    }
    mNumDirtyGuardPages = 0;
    BlockSize alignmentOffset = (M_PAGE_SIZE - size_t(block) % M_PAGE_SIZE) % M_PAGE_SIZE;
    assert(size_t(block + alignmentOffset) % M_PAGE_SIZE == 0);


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

    bool yep = p == p166;
    char* origPtr = (char*)p;
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

    if(yep) std::cout << "blocksize: " << blockSize << std::endl;

    char* block = ptr - offset;
    bool found = false;
    for(size_t i = 0; i < numAllocatedPtrs; i++) {
        if(allocatedPtrs[i] == block) {
            found = true;
            break;
        }
    }
    assert(found);
    BlockSize alignmentOffset = (M_PAGE_SIZE - size_t(block) % M_PAGE_SIZE) % M_PAGE_SIZE;
    if(M_CHECK_OVERRUN) {
        VirtualProtect(block + alignmentOffset + blockSize - M_PAGE_SIZE * 2, M_PAGE_SIZE, PAGE_READWRITE, &prevProtect);
    }

    toggleAllocating(true);
    mAllocator.dealloc_internal(block);
    toggleAllocating(false);
    for(char* page = block + alignmentOffset; page <= block + alignmentOffset + blockSize; page += M_PAGE_SIZE) {
//        assert(mNumGuardPages < M_NUM_PAGES);
//        mGuardPages[mNumGuardPages] = page;
//        mNumGuardPages++;

        if(page < mPageHolder.minPage) {
            mPageHolder.minPage = page;
        }
        else if(page > mPageHolder.maxPage) {
            mPageHolder.maxPage = page;
        }
    }
    size_t blockBeginPage = size_t(block) / M_PAGE_SIZE;
    size_t blockEndPage = size_t(block + blockSize) / M_PAGE_SIZE;
    bool merpFound = false;
    for(size_t i = 0; i < mNumDirtyGuardPages; i++) {
            if(size_t(mDirtyGuardPages[i]) / M_PAGE_SIZE == size_t(p166) / M_PAGE_SIZE) {
                merpFound = true;
            }
//        size_t dirtyPage = size_t(mDirtyGuardPages[i]) / M_PAGE_SIZE;
//        if(dirtyPage < blockBeginPage || dirtyPage >= blockEndPage) {
            VirtualProtect(mDirtyGuardPages[i], 1, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
//        }
    }
    if(aboooo) {
        assert(merpFound);
        aboooo = false;
    }
    mNumDirtyGuardPages = 0;

//    if(p166) std::cout << __LINE__ << std::endl;
    VirtualProtect(block + alignmentOffset, blockSize - M_PAGE_SIZE, PAGE_READWRITE | PAGE_GUARD, &prevProtect);

//    if(p166) std::cout << __LINE__ << std::endl;

    VirtualProtect(origPtr, 1, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
    assert(prevProtect == PAGE_READWRITE | PAGE_GUARD);



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
