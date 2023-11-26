/* author: cyberchriz(Christian Suer)

USAGE:
copy this file into the headers folder of the project,
then add the following preprocessor directives to the beginning in the file that is supposed logged:

        #include "timer.h"
        #define TIMELOG // comment this line out in order to deactivate logging

then use the macro 'TIMER_START' anywhere in the code to start a timer;
--> it will print its lifetime on the console once it goes out of scope,
i.e. as soon as the function or scope it lives in ends
--> alternatively, it will output the elapsed time if the macro TIMER_STOP is used

after a timer has been started, you can also use the macro TIMER_NOW at any time
in order to query the elapsed time (µs) since timer start */

#pragma once
#include <chrono>
#include <iostream>
#inlude "log.h"

#ifdef TIMELOG
#define TIMER_START Timer timer;
#define TIMER_NOW timer.elapsed_microsec()
#define TIMER_STOP timer.stop();
#else
#define TIMER_START
#define TIMER_NOW 0.0f // = default microsec in case no timer was started (change value if needed)
#define TIMER_STOP
#endif

class Timer {
public:
    double elapsed_microsec(){
        end = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::duration<double>>(end - start)).count() / 1000;
    }           

    // constructor
    Timer() = delete;
    Timer(std::string caller_function="<...>") {
        start = std::chrono::high_resolution_clock::now();
        Log::log(LOG_LEVEL_INFO, "timer started in scope ", caller_function);
    }
    double stop() {
        Log::log(LOG_LEVEL_INFO, "... timer stopped after ", elapsed_microsec(), " µs");
        stopped = true;
    }
private:
    // destructor
    ~Timer() {
        if (!stopped) {
            Log::log(LOG_LEVEL_INFO, "... end of timer scope: ", elapsed_microsec(), " µs");
        }
    }
    std::chrono::high_resolution_clock::time_point start, end;
    bool stopped = false;
};