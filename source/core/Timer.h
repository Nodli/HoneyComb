#ifndef H_TIMER
#define H_TIME

#include <chrono>

struct Timer{

    void start();
    void pause();
    void resume();

    typedef std::nano nano;
    typedef std::micro micro;
    typedef std::milli milli;
    typedef std::ratio<1> seconds;
    typedef std::ratio<60> minutes;
    typedef std::ratio<3600> hours;
    template<typename ReturnType, typename DurationPeriod>
    ReturnType get();

    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point reference;
};

template<typename ReturnType, typename DurationPeriod>
ReturnType Timer::get(){
    using duration_type = std::chrono::duration<ReturnType, DurationPeriod>;

    std::chrono::high_resolution_clock::time_point now = clock.now();
    duration_type elapsed = now - reference;

    return elapsed.count();
}

#endif
