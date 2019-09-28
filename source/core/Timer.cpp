#include "Timer.h"

void Timer::start(){
    reference = clock.now();
}

void Timer::pause(){
    using duration_type = std::chrono::duration<float, std::nano>;
    duration_type elapsed_since_start = clock.now() - reference;

    reference = std::chrono::high_resolution_clock::time_point()
        + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(elapsed_since_start);
}

void Timer::resume(){
    using duration_type = std::chrono::duration<float, std::nano>;
    duration_type elapsed_since_pause = clock.now() - reference;

    reference = std::chrono::high_resolution_clock::time_point()
        + std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(elapsed_since_pause);
}
