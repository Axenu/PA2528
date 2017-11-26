#ifndef __ALLOCATOR_BASE__
#define __ALLOCATOR_BASE__

#include <iostream>
#include <cstdlib>

#include "MemoryTracker.h"

//#define ENABLE_STOMP true
//#define TRACK_MEMORY true
#define ENABLE_ALLOC_DEBUG

class StompAllocator;

#ifdef ENABLE_ALLOC_DEBUG
//#define ALLOC_DEBUG(t, args) std::cout << __FILE__ << " (" << __LINE__ << ") " << #t#args << " -- size " << sizeof(t) << std::endl
#define ALLOC_DEBUG(t, args) MemoryTracker::out() << __FILE__ << " (" << __LINE__ << ") " << #t#args << " -- size " << sizeof(t) << std::endl
#else
#define ALLOC_DEBUG(t, args)
#endif // ENABLE_ALLOC_DEBUG

// Example usage:
// DefaultAllocator allocator;
// char a = allocator.ALLOC(char, ());
// ...
// Or:
// DefaultAllocator allocator;
// Foo a = allocator.ALLOC(Foo, (arg1, arg2, arg3));
#define ALLOC(t, args) alloc<t>(args); ALLOC_DEBUG(t, args);

class AllocatorBase {
public:
    AllocatorBase();
    #ifdef ENABLE_STOMP
    struct StompFlag {};
    AllocatorBase(StompFlag stompFlag);
    #endif // ENABLE_STOMP
    virtual ~AllocatorBase();

	// Sets the name for this allocator in the memory tracker
	void setTrackingName(std::string name);

    template <typename T, typename... Args>
    T* alloc(Args... args) {
        // std::cout << file << ": " << row << std::endl;
        #ifdef ENABLE_STOMP
        T* memory = (T*)mStompAllocator->alloc_internal(sizeof(T));
        #else
        T* memory = (T*)this->alloc_internal(sizeof(T));
        #endif // ENABLE_STOMP

		#ifdef TRACK_MEMORY
		MemoryTracker::allocDependent(memory, sizeof(T), ID);
		#endif

        return new(memory) T(args...);
    }
    template <typename T, typename... Args>
    T* alloc_arr(size_t size, Args... args) {
        // std::cout << "Alloc_arr with size: " << (sizeof(T)*size) << std::endl;
        #ifdef ENABLE_STOMP
        T* memory = (T*)mStompAllocator->alloc_internal(sizeof(T)*size);
        #else
        T* memory = (T*)this->alloc_internal(sizeof(T)*size);
        #endif // ENABLE_STOMP

		#ifdef TRACK_MEMORY
		MemoryTracker::allocDependent(memory, sizeof(T) * size, ID);
		#endif

        for (int i = 0; i < size; i++) {
            new(&memory[i]) T(args...);
        }
        return memory;
    }
    template <typename T>
    void dealloc(T *t) {
        // std::cout << file << ": " << row << std::endl;
        // t->~T();
        #ifdef ENABLE_STOMP
        mStompAllocator->dealloc_internal(t);
        #else
        this->dealloc_internal(t);
        #endif // ENABLE_STOMP

		#ifdef TRACK_MEMORY
		MemoryTracker::deallocDependent(t);
		#endif
    }
private:
    virtual void *alloc_internal(size_t size) = 0;
    virtual void dealloc_internal(void *p) = 0;

	// Holds the next free ID for use in memory tracking
	static size_t nextFreeID;

protected:
	// Memory tracking ID for this allocator
	const size_t ID;

private:
    #ifdef ENABLE_STOMP
    AllocatorBase* mStompAllocator;
    #endif // ENABLE_STOMP
    friend StompAllocator;
};

#endif
