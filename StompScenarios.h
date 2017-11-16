#ifdef __WIN32
#ifndef __STOMP_SCENARIOS__
#define __STOMP_SCENARIOS__

class AllocatorBase;

void stompOverrunFailScenario(AllocatorBase* currentGlobalAllocator);
void stompUnderrunFailScenario(AllocatorBase* currentGlobalAllocator);
void stompPassScenario(AllocatorBase* currentGlobalAllocator);
void runStompScenarios();

#endif
#endif
