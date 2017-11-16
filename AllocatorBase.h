#ifndef __ALLOCATOR_BASE__
#define __ALLOCATOR_BASE__

#include <iostream>
#include <cstdlib>

//#define ENABLE_STOMP

#ifdef ENABLE_STOMP
class StompAllocator;
#endif // ENABLE_STOMP

class AllocatorBase {
public:
    AllocatorBase();
    #ifdef ENABLE_STOMP
    struct StompFlag {};
    AllocatorBase(StompFlag stompFlag);
    #endif // ENABLE_STOMP
    virtual ~AllocatorBase();

    template <typename T, typename... Args>
    T* alloc(Args... args) {
        // std::cout << file << ": " << row << std::endl;
        #ifdef ENABLE_STOMP
        T* memory = (T*)mStompAllocator->alloc_internal(sizeof(T));
        #else
        T* memory = (T*)this->alloc_internal(sizeof(T));
        #endif // ENABLE_STOMP
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
        for (int i = 0; i < size; i++) {
            new(&memory[i]) T(args...);
        }
        return memory;
    }
    template <typename T>
    void dealloc(T *t) {
        // std::cout << file << ": " << row << std::endl;
        t->~T();
        #ifdef ENABLE_STOMP
        mStompAllocator->dealloc_internal(t);
        #else
        this->dealloc_internal(t);
        #endif // ENABLE_STOMP
    }
private:
    virtual void *alloc_internal(size_t size) = 0;
    virtual void dealloc_internal(void *p) = 0;


private:
    #ifdef ENABLE_STOMP
    AllocatorBase* mStompAllocator;
    friend StompAllocator;
    #endif // ENABLE_STOMP
};

#endif
