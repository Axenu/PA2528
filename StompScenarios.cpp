#ifdef __WIN32
#include "StompScenarios.h"
#include "StompAllocator.h"

#include <windows.h>
#include <signal.h>
#include <csetjmp>

namespace STOMP_SCENARIOS_PRIVATE {
    bool DO_EXPECT_SIGSEGV = false;
    char* PTR = nullptr;
    const char* FUNCTION = nullptr;
    int LINE = 0;
    bool GOT_SIGSEGV = false;
    static constexpr size_t PAGE_SIZE_MULTIPLIER = 2;
    jmp_buf JMP_ENV;
}
using namespace STOMP_SCENARIOS_PRIVATE;

#define PRINT_UNRECOVERABLE


#define PRINT_FAIL std::cout << "FAIL: " << __FUNCTION__ << " (" << __LINE__ << ")" << std::endl
#define PRINT_PASS std::cout << "PASS: " << __FUNCTION__ << std::endl

#define SET_HANDLER_DATA LINE = __LINE__; FUNCTION = __FUNCTION__;
#define EXPECT_SIGSEGV(op) SET_HANDLER_DATA; DO_EXPECT_SIGSEGV = true; GOT_SIGSEGV = false; if(!setjmp(JMP_ENV)) {op;} if(!GOT_SIGSEGV) { PRINT_FAIL; return; }\
\
//{ DWORD prevProtect; VirtualProtect(PTR, 1, PAGE_NOACCESS, &prevProtect);} \

#define EXPECT_NO_SIGSEGV(op) SET_HANDLER_DATA; DO_EXPECT_SIGSEGV = false; GOT_SIGSEGV = false; if(!setjmp(JMP_ENV)) {op;} if(GOT_SIGSEGV) {GOT_SIGSEGV = false; PRINT_FAIL; return;}

char pleasework;
AllocatorBase* merp;
void sigsegvHandler(int sig) {
//    std::cout << __FUNCTION__ << std::endl;
    signal(SIGSEGV, sigsegvHandler);
    if(sig == SIGSEGV) {
//        std::cout << "sigsegv" << std::endl;
        GOT_SIGSEGV = true;
        longjmp(JMP_ENV, 0);
    }
}

typedef void (*SignalHandler)(int);


void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        PTR = a + size;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }

    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        PTR = a + size;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        allocs[size - 1] = a;
    }

    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    EXPECT_NO_SIGSEGV(cga->dealloc((uint32_t*)allocs););
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        PTR = a - 1;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }
    char** allocs;
    EXPECT_NO_SIGSEGV(allocs = cga->alloc_arr<char*>(StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER - 1));
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        PTR = a - 1;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        allocs[size - 1] = a;
    }

    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        EXPECT_NO_SIGSEGV(cga->dealloc(allocs[size - 1]););
    }
    EXPECT_NO_SIGSEGV(cga->dealloc((uint32_t*)allocs););
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
    EXPECT_NO_SIGSEGV(cga->dealloc((uint32_t*)allocs););
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompAccessFreedFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, sigsegvHandler);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a;
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(size););
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
        PTR = a;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
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
        PTR = a;
        char b;
        EXPECT_SIGSEGV(b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        EXPECT_NO_SIGSEGV(a = cga->alloc_arr<char>(1););
        EXPECT_NO_SIGSEGV(cga->dealloc(a););
    }
    signal(SIGSEGV, prevHandler);

    printf("%d\n", __LINE__);
    int a;
    StompAllocator::p166 = (char*)allocs;
    EXPECT_NO_SIGSEGV(cga->dealloc((uint32_t*)allocs););
    printf("%d", __LINE__);
    printf("%d", __LINE__);
    PRINT_PASS;
}

class MallocAllocator : public AllocatorBase {

private:
    void* alloc_internal(size_t size) override {
        return malloc(size);
    }

    void dealloc_internal(void* p) override {
        return free(p);
    }
};

#include "StompAllocator.h"
void runStompScenarios()
{
    MallocAllocator allocator;
//    PoolAllocator allocator(1000, 1000);
//    PoolAllocator allocator(sizeof(uint32_t) * StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER, 10000);
    {
        std::cout << "Running StompAllocator overrun tests..." << std::endl;
        StompAllocator overrun(allocator, true);
//        merp = &overrun;
        stompAccessFreedFailScenario(&overrun);
        stompPassScenario(&overrun);
        stompOverrunFailScenario(&overrun);
    }

    {
        std::cout << "Running StompAllocator underrun tests..." << std::endl;
        StompAllocator underrun(allocator, false);
        stompAccessFreedFailScenario(&underrun);
        stompPassScenario(&underrun);
        stompUnderrunFailScenario(&underrun);

        std::cout << __LINE__ << std::endl;
//        delete underrun;
        std::cout << __LINE__ << std::endl;
    }

    std::cout << __LINE__ << std::endl;
//    delete allocator;
    std::cout << __LINE__ << std::endl;
}
#endif
