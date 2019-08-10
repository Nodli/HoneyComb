#ifndef H_LINEARALLOCATOR
#define H_LINEARALLOCATOR

struct LinearAllocator{

    LinearAllocator();
    ~LinearAllocator();

    void allocate(size_t bytes);
    void deallocate();
    void reset();

    template<typename T, typename ... ConstructorArguments>
    T* get(const ConstructorArguments& ... arguments);

    // ---- Data ---- //

    size_t memory_bytes = 0;
    void* memory = nullptr;
    char* current = nullptr;
}

#endif
