#if defined(__WIN32)  || defined(WIN32) || defined(_WIN32)
#ifndef __STOMP_SCENARIOS__
#define __STOMP_SCENARIOS__

class AllocatorBase;

void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator);
void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator);
void stompPassScenario(AllocatorBase* currentGlobalAllocator);
void stompAccessFreedFailScenario(AllocatorBase* currentGlobalAllocator);

#endif
#endif
