#if defined(__WIN32)  || defined(WIN32) || defined(_WIN32)
#include "StompScenarios.h"
#include "StompAllocator.h"

#include <windows.h>
#include <signal.h>
#include <csetjmp>

namespace STOMP_SCENARIOS_PRIVATE {
    bool GOT_SIGSEGV = false;
    static constexpr size_t PAGE_SIZE_MULTIPLIER = 2;
    jmp_buf JMP_ENV;
}
using namespace STOMP_SCENARIOS_PRIVATE;

#define PRINT_FAIL std::cout << "FAIL: " << __FUNCTION__ << " (" << __LINE__ << ")" << std::endl
#define PRINT_PASS std::cout << "PASS: " << __FUNCTION__ << std::endl

#define TRY_OP(op) if(!setjmp(JMP_ENV)) {op;}
#define EXPECT_SIGSEGV(op) GOT_SIGSEGV = false; TRY_OP(op); if(!GOT_SIGSEGV) { PRINT_FAIL; return; }
#define EXPECT_NO_SIGSEGV(op) GOT_SIGSEGV = false; TRY_OP(op); if(GOT_SIGSEGV) {GOT_SIGSEGV = false; PRINT_FAIL; return;}

void sigsegvHandler(int sig) {
    signal(SIGSEGV, sigsegvHandler);
    if(sig == SIGSEGV) {
        GOT_SIGSEGV = true;
        longjmp(JMP_ENV, 0);
    }
}

typedef void (*SignalHandler)(int);


void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    char* ptr;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        ptr = a + size;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }

    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        ptr = a + size;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
        allocs[size - 1] = a;
    }

    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    EXPECT_NO_SIGSEGV(cga->dealloc(allocs););
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    char* ptr;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        ptr = a - 1;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }
    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        ptr = a - 1;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
        allocs[size - 1] = a;
    }

    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    EXPECT_NO_SIGSEGV(cga->dealloc(allocs););
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompPassScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        EXPECT_NO_SIGSEGV(memcpy(a, a, size));
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }



    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        EXPECT_NO_SIGSEGV(memcpy(a, a, size));
        allocs[size - 1] = a;
    }

    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    EXPECT_NO_SIGSEGV(cga->dealloc(allocs););
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompAccessFreedFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    char* ptr;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
        ptr = a;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
    }

    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(allocs[size - 1] = cga->alloc_arr<char>(size););
    }
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a = allocs[size - 1];
        ptr = a;
        char b;
        EXPECT_SIGSEGV(b = *ptr);
        EXPECT_SIGSEGV(*ptr = b);
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(1););
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }
    signal(SIGSEGV, prevHandler);
    EXPECT_NO_SIGSEGV(cga->dealloc(allocs););
    PRINT_PASS;
}
#endif
