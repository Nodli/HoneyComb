/*

#include <cstdlib>

struct FreeList{

    struct list_element{
        list_element* next;
        size_t size;
    };

    template<typename T>
    union MemoryBlock{
        T data;
        list_element;
    };

    FreeList();
    ~FreeList();

    void allocate(size_t bytes);
    void deallocate();

    template<typename T>
    T* get();

    template<typename T>
    void give(T* ptr);

    void* memory = nullptr;
    list_element* current_block = nullptr;
    int arena_bytes = 0;
};

*/
