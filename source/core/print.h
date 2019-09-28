#ifndef H_PRINT
#define H_PRINT

// default entry point
template<typename T, typename ... otherT>
void print(const T& value, const otherT&... otherValue);

// char pointer entry point
template<typename ... otherT>
void print(const char* ptr, const otherT&... otherValue);

// base case
inline void print();

#include "print.inl"

#endif
