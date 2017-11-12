#ifndef __ALLOCATOR_BASE__
#define __ALLOCATOR_BASE__

#include <iostream>
#include <cstdlib>

class AllocatorBase {
public:
    AllocatorBase();
    virtual ~AllocatorBase();

    template <typename T, typename... Args>
    T* alloc(Args... args) {
        // std::cout << file << ": " << row << std::endl;
        T* memory = (T*)this->alloc_internal(sizeof(T));
        return new(memory) T(args...);
    }
    template <typename T, typename... Args>
    T* alloc_arr(size_t size, Args... args) {
        std::cout << "Alloc_arr with size: " << (sizeof(T)*size) << std::endl;
        T* memory = (T*)this->alloc_internal(sizeof(T)*size);
        for (int i = 0; i < size; i++) {
            new(&memory[i]) T(args...);
        }
        return memory;
    }
    template <typename T>
    void dealloc(T *t) {
        // std::cout << file << ": " << row << std::endl;
        t->~T();
        this->dealloc_internal(t);
    }
private:
    virtual void *alloc_internal(size_t size) = 0;
    virtual void dealloc_internal(void *p) = 0;
};

#endif
