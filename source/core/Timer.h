#ifndef H_TIMER
#define H_TIME

#include <chrono>

struct Timer{

    // Set the reference time to the current time
    void set();

    float nanoseconds();
    float microseconds();
    float milliseconds();
    float seconds();
    float minutes();
    float hours();

    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point reference;
};

#endif
