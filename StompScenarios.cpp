#include "StompScenarios.h"
#include "StompAllocator.h"

#include <windows.h>
#include <signal.h>

namespace STOMP_SCENARIOS_PRIVATE {
    char* PTR = nullptr;
    bool GOT_SIGSEGV = false;
    static constexpr size_t PAGE_SIZE_MULTIPLIER = 2;
}
using namespace STOMP_SCENARIOS_PRIVATE;

void signalHandlerExpectSigsegv(int sig) {
    if(sig == SIGSEGV) {
        GOT_SIGSEGV = true;
        DWORD prevProtect;
        VirtualProtect(PTR, 1, PAGE_READWRITE, &prevProtect);
        signal(SIGSEGV, signalHandlerExpectSigsegv);
    }
}

void signalHandlerNoExpectSigsegv(int sig) {
    if(sig == SIGSEGV) {
        GOT_SIGSEGV = true;
        signal(SIGSEGV, signalHandlerNoExpectSigsegv);
    }
}
typedef void (*SignalHandler)(int);

#define PRINT_FAIL std::cout << "FAIL: " << __FUNCTION__ << " (" << __LINE__ << ")" << std::endl
#define PRINT_PASS std::cout << "PASS: " << __FUNCTION__ << std::endl

#define EXPECT_SIGSEGV(op) op; if(!GOT_SIGSEGV) { PRINT_FAIL; return; }\
GOT_SIGSEGV = false; \
{ DWORD prevProtect; VirtualProtect(PTR, 1, PAGE_NOACCESS, &prevProtect);} \

#define EXPECT_NO_SIGSEGV(op) op; if(GOT_SIGSEGV) { PRINT_FAIL; GOT_SIGSEGV = false; return; }\

void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, signalHandlerExpectSigsegv);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a = cga->alloc_arr<char>(size);
        PTR = a + size;
        EXPECT_SIGSEGV(char b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        cga->dealloc(a);
    }
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, signalHandlerExpectSigsegv);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a = cga->alloc_arr<char>(size);
        PTR = a - 1;
        EXPECT_SIGSEGV(char b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
        cga->dealloc(a);
    }
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompPassScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, signalHandlerNoExpectSigsegv);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a = cga->alloc_arr<char>(size);
        char* b = cga->alloc_arr<char>(size);
        EXPECT_NO_SIGSEGV(memcpy(a, b, size));
        cga->dealloc(a);
        cga->dealloc(b);
    }
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}

void stompAccessFreedFailScenario(AllocatorBase* currentGlobalAllocator) {
    SignalHandler prevHandler = signal(SIGSEGV, signalHandlerExpectSigsegv);
    AllocatorBase* cga = currentGlobalAllocator;
    for(size_t size = 1; size < StompAllocator::getPageSize() * PAGE_SIZE_MULTIPLIER; size++) {
        char* a = cga->alloc_arr<char>(size);
        cga->dealloc(a);
        PTR = a;
        EXPECT_SIGSEGV(char b = *PTR);
        EXPECT_SIGSEGV(*PTR = b);
    }
    signal(SIGSEGV, prevHandler);
    PRINT_PASS;
}
#include "StompAllocator.h"
void runStompScenarios()
{
    {
        std::cout << "Running StompAllocator overrun tests..." << std::endl;
        StompAllocator overrun(true);
        stompAccessFreedFailScenario(&overrun);
        stompPassScenario(&overrun);
        stompOverrunFailScenario(&overrun);
    }

    {
        std::cout << "Running StompAllocator underrun tests..." << std::endl;
        StompAllocator underrun(false);
        stompAccessFreedFailScenario(&underrun);
        stompPassScenario(&underrun);
        stompUnderrunFailScenario(&underrun);
    }

}
