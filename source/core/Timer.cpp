#include "Timer.h"

void Timer::set(){
    reference = clock.now();
}

float Timer::nanoseconds(){

    using duration_type = std::chrono::duration<float, std::nano>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

float Timer::microseconds(){

    using duration_type = std::chrono::duration<float, std::micro>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

float Timer::milliseconds(){

    using duration_type = std::chrono::duration<float, std::milli>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

float Timer::seconds(){

    using duration_type = std::chrono::duration<float>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

float Timer::minutes(){

    using duration_type = std::chrono::duration<float, std::ratio<60>>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

float Timer::hours(){

    using duration_type = std::chrono::duration<float, std::ratio<3600>>;
    static_assert(std::chrono::treat_as_floating_point<duration_type::rep>::value);

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}
