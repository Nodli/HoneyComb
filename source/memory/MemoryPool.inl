#ifndef INL_MEMORYPOOL
#define INL_MEMORYPOOL

template<size_t block_size>
MemoryPool<block_size>::MemoryPool(){
}

template<size_t block_size>
MemoryPool<block_size>::~MemoryPool(){
    free(memory);
}

template<size_t block_size>
void MemoryPool<block_size>::allocate(int number_of_blocks){
    free(memory);
    memory = malloc(number_of_blocks * sizeof(MemoryBlock));
    current_block = (MemoryBlock*)memory;

    for(size_t block = 0; block != number_of_blocks - 1; ++block){
        (current_block + block)->next = current_block + (block + 1);
    }

    free_blocks = number_of_blocks;
}

template<size_t block_size>
void MemoryPool<block_size>::deallocate(){
    free(memory);
    memory = nullptr;
    current_block = nullptr;
    free_blocks = 0;
}

template<size_t block_size>
template<typename T, typename ... ConstructorArguments>
T* MemoryPool<block_size>::get(const ConstructorArguments& ... arguments){

    static_assert(sizeof(T) <= block_size && (block_size % std::alignment_of(T)) == 0,
                "The alignment of T is not compatible with the block_size of the MemoryPool");

    if(free_blocks){
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
    if(ptr){
        ptr->~T();
        ((MemoryBlock*)ptr)->next = current_block;
        current_block = (MemoryBlock*)ptr;
        ++free_blocks;
    }
}

#endif
