#if defined(__WIN32)  || defined(WIN32) || defined(_WIN32)

#include "StompAllocator.h"

#include <iostream>
#include <cstdlib>
#include <malloc.h>
#include <windows.h>
#include <cassert>


namespace STOMP_ALLOCATOR_PRIVATE {
LONG CALLBACK guardPageHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    EXCEPTION_RECORD& er = *ExceptionInfo->ExceptionRecord;
    if(er.ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        void* ptr = (void*)er.ExceptionInformation[1];
        if(!StompAllocator::isAllocating()) {
            DWORD prevProtect;
            VirtualProtect(ptr, 1, PAGE_NOACCESS, &prevProtect);
        }
        StompAllocator::addDirtyGuardPage(ptr);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef ENABLE_STOMP
class DefStompAllocator : public AllocatorBase {
public:
    DefStompAllocator() : AllocatorBase(StompFlag()){
    }

    virtual ~DefStompAllocator() {
    }

private:
    void *alloc_internal(size_t size) {
        return malloc(size);
    }
    void dealloc_internal(void *p) {
        free(p);
    }
};

    DefStompAllocator globalAllocator;
    StompAllocator globalStompAllocator(globalAllocator, ENABLE_STOMP);
#endif // ENABLE_STOMP
}
using namespace STOMP_ALLOCATOR_PRIVATE;

#ifdef ENABLE_STOMP
void* operator new(size_t count) {
    return globalStompAllocator.alloc_arr<char>(count);
}

void* operator new[](size_t count) {
    return globalStompAllocator.alloc_arr<char>(count);
}

void  operator delete(void* ptr) {
    globalStompAllocator.dealloc(ptr);
}

void  operator delete[](void* ptr) {
    globalStompAllocator.dealloc(ptr);
}
#endif // ENABLE_STOMP


const size_t StompAllocator::M_PAGE_SIZE = []() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwPageSize;
}();

StompAllocator* StompAllocator::mStompAllocators;
size_t StompAllocator::mNumInstances = 0;
StompAllocator::PageHolder StompAllocator::mPageHolder;


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
}

StompAllocator::PageHolder::PageHolder() {
    mStompAllocators = nullptr;
    vehHandle = AddVectoredExceptionHandler(0, guardPageHandler);

    int a;
    allocator = new StompAllocator(*(AllocatorBase*)&a, true);
}

StompAllocator::PageHolder::~PageHolder() {
    for(char* p = (char*)minPage; p <= maxPage; p += M_PAGE_SIZE) {
        DWORD prevProtect;
        VirtualProtect(p, 1, PAGE_READWRITE, &prevProtect);
    }

    RemoveVectoredExceptionHandler(vehHandle);
    delete allocator;
}

StompAllocator::~StompAllocator() {
    mNumInstances--;
    if(mNumInstances == 1) {
        mStompAllocators = mPageHolder.allocator;
    }
}

size_t StompAllocator::getPageSize() {
    return M_PAGE_SIZE;
}


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

size_t StompAllocator::getAllocSize(size_t size) {
    size += sizeof(BlockSize) * 2;
    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    return size + M_PAGE_SIZE * 2 + distanceToPageBoundary;
}


void* StompAllocator::alloc_internal(size_t size) {
    if(M_CHECK_OVERRUN) {
        size += sizeof(BlockSize) * 2;
    }

    size_t distanceToPageBoundary = (M_PAGE_SIZE - size % M_PAGE_SIZE) % M_PAGE_SIZE;
    BlockSize blockSize = size + M_PAGE_SIZE * 2 + distanceToPageBoundary;

    toggleAllocating(true);
    char* block = (char*)mAllocator.alloc_internal(blockSize);
    toggleAllocating(false);

    DWORD prevProtect;
    for(size_t i = 0; i < mNumDirtyGuardPages; i++) {
        VirtualProtect(mDirtyGuardPages[i], 1, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
    }
    mNumDirtyGuardPages = 0;

    BlockSize alignmentOffset = (M_PAGE_SIZE - size_t(block) % M_PAGE_SIZE) % M_PAGE_SIZE;

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

    toggleAllocating(true);
    mAllocator.dealloc_internal(block);
    toggleAllocating(false);

    for(size_t i = 0; i < mNumDirtyGuardPages; i++) {
        VirtualProtect(mDirtyGuardPages[i], 1, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
    }
    mNumDirtyGuardPages = 0;

    for(char* page = block + alignmentOffset; page <= block + alignmentOffset + blockSize; page += M_PAGE_SIZE) {
        if(page < mPageHolder.minPage) {
            mPageHolder.minPage = page;
        }
        else if(page > mPageHolder.maxPage) {
            mPageHolder.maxPage = page;
        }
    }
    VirtualProtect(block + alignmentOffset, blockSize - M_PAGE_SIZE, PAGE_READWRITE | PAGE_GUARD, &prevProtect);
}

#endif // __WIN32
