#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"


class StompAllocator : public AllocatorBase {
public:
    static char* p166;
    StompAllocator(AllocatorBase& allocator, bool doCheckOverrun);
    ~StompAllocator();

    static size_t getPageSize();
    static bool isAllocating();
    static void addDirtyGuardPage(void* p);
    static bool isActive();

private:
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);

    void toggleAllocating(bool flag);

private:
    using BlockSize = size_t;
    struct Block {
        BlockSize size;
        char* ptr;
    };
    static const size_t M_PAGE_SIZE;
    const bool M_CHECK_OVERRUN;
    AllocatorBase& mAllocator;
    Block* mFreeBlocks;
    size_t mNumFreeBlocks;
    static constexpr size_t M_NUM_THREADS = 1024;
    static constexpr size_t M_NUM_PAGES = 500000;
    static StompAllocator* mStompAllocators;
    void** mDirtyGuardPages;
    size_t mNumDirtyGuardPages = 0;
    void* mGuardPages[M_NUM_PAGES];
    size_t mNumGuardPages = 0;
    volatile bool mIsAllocating = false;
};

#endif
