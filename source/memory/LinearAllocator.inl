#ifndef INL_LINEARALLOCATOR
#define INL_LINEARALLOCATOR

template<typename T, typename ... ConstructorArguments>
T* LinearAllocator::get(const ConstructorArguments& ... arguments){

    char* aligned_ptr = current + (current % std::alignment_of(T));

    if((memory_bytes - (aligned_ptr - memory)) >= sizeof(T)){
        current = aligned_ptr;
        return new (aligned_ptr) T(arguments...);
    }else{
        return nullptr;
    }
}

#endif
