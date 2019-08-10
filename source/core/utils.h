#ifndef H_UTILS
#define H_UTILS

#include <string>
#include <iostream>

// ---- C Random numbers ---- //

float rand_normalized();

// ---- Hashing functions ---- //

// https://gist.github.com/badboy/6267743
// http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
constexpr int hash32shift(int key);
constexpr int hash32shiftmult(int key);
constexpr long hash64shift(long key);

// ---- Math utilities ---- //

// -1 if number < 0
// 0 if number == 0
// 1 if number > 0
template<typename T>
constexpr T sign(const T number);

template<typename T>
constexpr bool is_power_of_two(T number);

template<typename T>
constexpr T modulo_two_pi(const T number);

// ---- IO Files ---- //

void read_file(const char* const file_path, std::string& output);
std::string read_file(const char* const file_path);

template<typename T>
void put_bytes(std::ostream& stream, const T val);

template<typename T>
T get_bytes(std::istream& stream);

// ---- Concatenation to string ---- //

#define SHOW_CONCATENATE_INTERNAL_PATH false

// default entry point
template<typename T, typename ... otherT>
std::string concatenate(const T& value, const otherT&... otherValue);

// char pointer entry point
template<typename T, typename ... otherT>
std::string concatenate(const char* ptr, const otherT&... otherValue);

// intermediate default case
template<typename T, typename ... otherT>
void concatenate_step(std::string& output, const T& value, const otherT&... otherValue);

// intermediate when a conversion to std::string is not requiered
template<typename ... otherT>
void concatenate_step(std::string& output, const char* ptr, const otherT&... otherValue);

template<typename ... otherT>
void concatenate_step(std::string& output, const std::string& str, const otherT&... otherValue);

template<typename ... otherT>
void concatenate_step(std::string& string, const char c, const otherT&... otherValue);

// base case
inline void concatenate_step(const std::string& output);

#include "utils.inl"

#endif
