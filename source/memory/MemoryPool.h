#ifndef H_MEMORYPOOL
#define H_MEMORYPOOL

#include <cstdlib>

// block_size should be determined using sizeof(T) for the type we want to store
// in the MemoryPool because this makes sure blocks are properly aligned
template<size_t block_size>
struct MemoryPool{

    // ---- Constructor ---- //

    MemoryPool();
    ~MemoryPool();

    // ---- Allocation ---- //

    void allocate(const size_t blocks);
    void deallocate();
    void reset();

    // ---- Block get / give ---- //

    // Takes a block from the pool and calls the constructor of T inside it using a placement new
    // Returns nullptr when there are no blocks available
    // ex:
    // A{short, short, short}   ; sizeof(A) = 6 and alignment on a multiple of 2
    // B{int}                   ; sizeof(B) = 4 and alignment on a multiple of 4
    // sizeof(B) < sizeof(A) so we *could* fit a B inside a block from a MemoryPool<sizeof(A)>
    // However the blocks adresses are : 0 6 12 18 ... which are not all multiples of 4
    // The placement new at a non-aligned adress is undefined behavior so we do
    // not allow the allocation of a B inside a MemoryPool<sizeof(A)>
    template<typename T, typename ... ConstructorArguments>
    T* get(const ConstructorArguments& ... arguments);

    // Calls the destructor for T and returns the block to the pool
    template<typename T>
    void give(T* ptr);

    // ---- Data ---- //

    union MemoryBlock{
        char data[block_size];
        MemoryBlock* next;
    };

    void* memory = nullptr;
    size_t memory_blocks = 0;
    MemoryBlock* current_block = nullptr;
};

#include "MemoryPool.inl"

#endif
