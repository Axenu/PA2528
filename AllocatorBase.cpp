#include "AllocatorBase.h"

#ifdef ENABLE_STOMP
#include "StompAllocator.h"
#endif // ENABLE_STOMP

size_t AllocatorBase::nextFreeID = 0;

AllocatorBase::AllocatorBase() : ID{ nextFreeID } {
    // std::cout << "ctor AllocatorBase" << std::endl;

	++nextFreeID;

    #ifdef ENABLE_STOMP
    mStompAllocator = new StompAllocator(*this, ENABLE_STOMP);
    #endif // ENABLE_STOMP
}

#ifdef ENABLE_STOMP
AllocatorBase::AllocatorBase(StompFlag stompFlag) {
    // std::cout << "ctor AllocatorBase" << std::endl;
    mStompAllocator = this;
}
#endif // ENABLE_STOMP

AllocatorBase::~AllocatorBase() {
    // std::cout << "dtor AllocatorBase" << std::endl;

    #ifdef ENABLE_STOMP
    if(mStompAllocator != this) {
        delete mStompAllocator;
    }
    #endif // ENABLE_STOMP
}

void AllocatorBase::setTrackingName(std::string name)
{
	MemoryTracker::setAllocatorName(ID, name);
}
