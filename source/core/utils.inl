#ifndef INL_UTILS
#define INL_UTILS

#include <cmath>

template<typename T>
constexpr T sign(const T number){

    static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value);

	return (0.f < number) - (number < 0.f);
}

template<typename T>
constexpr bool is_power_of_two(T number){
    static_assert(std::is_integral<T>::value);
    return number && !(number & (number - 1));
}

template<typename T>
constexpr T modulo_two_pi(T number){

    constexpr bool use_branching_implementation = false;

    constexpr float two_pi = 2 * M_PI;
    if(use_branching_implementation){

        if(number > two_pi){
            return number - two_pi;
        }else if(number < 0){
            return number + two_pi;
        }

    }else{

        const int modulo_operation = (int)(number < 0)
            - (int)(number > two_pi);
        return number + modulo_operation * two_pi;

    }
}

template<typename T>
void put_bytes(std::ostream& stream, const T val){
    const char* const val_bytes = (const char*)(&val);
    stream.write(val_bytes, sizeof(T));
}

template<typename T>
T get_bytes(std::istream& stream){
    T val;
    char* const val_bytes = (char* const)(&val);
    stream.read(val_bytes, sizeof(T));

    return val;
}


template<typename T, typename ... otherT>
std::string concatenate(const T& value, const otherT&... otherValue){
    std::string output;
    concatenate_step(output, value);
    concatenate_step(output, otherValue...);

    return output;
}

template<typename T, typename ... otherT>
std::string concatenate(const char* ptr, const otherT&... otherValue){
    std::string output;
    concatenate_step(output, ptr);
    concatenate_step(output, otherValue...);

    return output;
}

template<typename T, typename ... otherT>
void concatenate_step(std::string& output, const T& value, const otherT&... otherValue){
    output += std::to_string(value);
    concatenate_step(output, otherValue...);
}

template<typename ... otherT>
void concatenate_step(std::string& output, const char* ptr, const otherT&... otherValue){
    output += ptr;
    concatenate_step(output, otherValue...);
}

template<typename ... otherT>
void concatenate_step(std::string& output, const std::string& str, const otherT&... otherValue){
    output.reserve(output.size() + str.size());
    output += str;
    concatenate_step(output, otherValue...);
}

template<typename ... otherT>
void concatenate_step(std::string& output, const char c, const otherT&... otherValue){
    output += c;
    concatenate_step(output, otherValue...);
}

inline void concatenate_step(const std::string& output){
}

#endif
