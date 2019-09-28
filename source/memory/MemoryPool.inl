#ifndef INL_MEMORYPOOL
#define INL_MEMORYPOOL

#include <cassert>

template<size_t block_size>
MemoryPool<block_size>::MemoryPool(){
}

template<size_t block_size>
MemoryPool<block_size>::~MemoryPool(){
    free(memory);
}

template<size_t block_size>
void MemoryPool<block_size>::allocate(const size_t blocks){
    free(memory);
    memory = malloc(blocks * sizeof(MemoryBlock));
    current_block = (MemoryBlock*)memory;

    for(size_t iblock = 0; iblock != blocks - 1; ++iblock){
        (current_block + iblock)->next = current_block + (iblock + 1);
    }

    memory_blocks = blocks;
}

template<size_t block_size>
void MemoryPool<block_size>::deallocate(){
    free(memory);
    memory = nullptr;
    current_block = nullptr;
}

template<size_t block_size>
void MemoryPool<block_size>::reset(){
    current_block = (MemoryBlock*)memory;

    for(size_t block = 0; block != memory_blocks - 1; ++block){
        (current_block + block)->next = current_block + (block + 1);

    }
}

template<size_t block_size>
template<typename T, typename ... ConstructorArguments>
T* MemoryPool<block_size>::get(const ConstructorArguments& ... arguments){

    static_assert(sizeof(T) <= block_size && (block_size % alignof(T)) == 0,
                "The alignment of T is not compatible with the block_size of the MemoryPool");

    if(current_block){
        // The temporary storage is necessary because of the placement new
        MemoryBlock* placement_ptr = current_block;
        current_block = current_block->next;
        return new (placement_ptr) T(arguments...);
    }else{
        return nullptr;
    }
}

template<size_t block_size>
template<typename T>
void MemoryPool<block_size>::give(T* ptr){

    assert((void*)ptr >= memory && (void*)ptr <= (void*)((MemoryBlock*)(memory) + memory_blocks));

    if(ptr){
        ptr->~T();
        ((MemoryBlock*)ptr)->next = current_block;
        current_block = (MemoryBlock*)ptr;
    }
}

#endif
