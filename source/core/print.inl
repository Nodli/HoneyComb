#ifndef INL_PRINT
#define INL_PRINT

#include <iostream>

template<typename T, typename ... otherT>
void print(const T& value, const otherT&... otherValue){
	std::cout << value << " ";

	print(otherValue...);
}

template<typename ... otherT>
void print(const char* ptr, const otherT&... otherValue){
	if(ptr != nullptr){
		std::cout << ptr << " ";
	}

	print(otherValue...);
}

inline void print(){
	std::cout << std::endl; // flushes the buffer
}

#endif
