#ifndef INL_PRINT
#define INL_PRINT

#include <iostream>

template<typename T, typename ... otherT>
void print(const T& value, const otherT&... otherValue){
#if SHOW_PRINT_INTERNAL_PATH
    std::cout << "print(T&, const otherT&)" << std::endl;
#endif
	std::cout << value << " ";

	print(otherValue...);
}

template<typename ... otherT>
void print(const char* ptr, const otherT&... otherValue){
#if SHOW_PRINT_INTERNAL_PATH
    std::cout << "print(const char*, const otherT&)" << std::endl;
#endif
	if(ptr != nullptr){
		std::cout << ptr << " ";
	}

	print(otherValue...);
}

inline void print(){
#if SHOW_PRINT_INTERNAL_PATH
    std::cout << "print()" << std::endl;
#endif
	std::cout << std::endl; // flushes the buffer
}

#endif
