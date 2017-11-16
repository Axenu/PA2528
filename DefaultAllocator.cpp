#include "DefaultAllocator.h"

DefaultAllocator::DefaultAllocator() {
}
DefaultAllocator::~DefaultAllocator() {
}

void *DefaultAllocator::alloc_internal(size_t size) {
    return malloc(size);
}
void DefaultAllocator::dealloc_internal(void *p) {
    free(p);
}
