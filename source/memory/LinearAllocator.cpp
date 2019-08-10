#include "LinearAllocator.h"

LinearAllocator::LinearAllocator(){
}

LinearAllocator::~LinearAllocator(){
    free(memory);
}

void LinearAllocator::allocate(size_t bytes){
    free(memory);
    memory_bytes = bytes;
    memory = malloc(bytes);
    current = (char*)memory;
}

void LinearAllocator::deallocate(){
    free(memory);
    memory_bytes = 0;
    memory = nullptr;
    current = nullptr;
}

void LinearAllocator::reset(){
    current = (char*)memory;
}
