/* author: cyberchriz(Christian Suer)

USAGE:
use the macro 'TIMER_START' (or simply 'TIMER') anywhere in the code to start a timer;
--> it will print its lifetime on the console once it goes out of scope,
i.e. as soon as the function or scope it lives in ends
--> alternatively, it will output the elapsed time if the macro TIMER_STOP is used

after a timer has been started, you can also use the macro TIMER_NOW at any time
in order to query the elapsed time (µs) since timer start */

#ifndef TIMELOG_H
#define TIMELOG_H

#include "log.h"
#include <chrono>
#include <string>

#define TIMER Timer timer(__FUNCTION__);
#define TIMER_START Timer timer(__FUNCTION__);
#define ELAPSED timer.elapsed_microsec()
#define TIMER_STOP timer.stop();


class Timer {
public:
    double elapsed_microsec(){
        end = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::duration<double>>(end - start)).count() / 1000;
    }           

    // constructor
    Timer(std::string caller_function="") : caller_function(caller_function) {
        start = std::chrono::high_resolution_clock::now();
        if (caller_function == "") {
            Log::log(INFO, "timer started");
        }
        else {
            Log::log(INFO, "timer started in scope ", caller_function);
        }
    }
    void stop() {
        if (caller_function == "") {
            Log::log(INFO, "timer stopped after ", elapsed_microsec(), " microsec");
        }
        else {
            Log::log(INFO, "timer in function ", caller_function, " stopped after ", elapsed_microsec(), " microsec");
        }
        stopped = true;
        start = std::chrono::high_resolution_clock::now();
    }

    // destructor
    ~Timer() {
        if (!stopped) {
            Log::log(INFO, "end of timer in scope ", caller_function, ": ", elapsed_microsec(), " microsec");
            stopped = true;
            start = std::chrono::high_resolution_clock::now();
        }
    }
private:
    std::chrono::high_resolution_clock::time_point start, end;
    bool stopped = false;
    std::string caller_function = "";
};

#endif