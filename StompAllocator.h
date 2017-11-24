#if defined(__WIN32)  || defined(WIN32) || defined(_WIN32)
#ifndef __STOMP_ALLOCATOR__
#define __STOMP_ALLOCATOR__

#include "AllocatorBase.h"


class StompAllocator : public AllocatorBase {
public:
    StompAllocator(AllocatorBase& allocator, bool doCheckOverrun);
    ~StompAllocator();

    static size_t getPageSize();
    static bool isAllocating();
    static void addDirtyGuardPage(void* p);
    static bool isActive();
    static size_t getAllocSize(size_t size);

private:
    virtual void* alloc_internal(size_t size);
    virtual void dealloc_internal(void *p);

    void toggleAllocating(bool flag);

private:
    struct PageHolder {
        PageHolder();
        ~PageHolder();
        StompAllocator* allocator;
        char* minPage = (char*)-1;
        char* maxPage = (char*)0;
        void* vehHandle;
    };

    static constexpr size_t M_NUM_PAGES = 10000;
    static const size_t M_PAGE_SIZE;
    static StompAllocator* mStompAllocators;
    static size_t mNumInstances;
    static PageHolder mPageHolder;

    using BlockSize = size_t;
    const bool M_CHECK_OVERRUN;
    AllocatorBase& mAllocator;
    void* mDirtyGuardPages[M_NUM_PAGES];
    size_t mNumDirtyGuardPages = 0;
    volatile bool mIsAllocating = false;

};

#endif
#endif
