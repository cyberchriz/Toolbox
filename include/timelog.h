// author: cyberchriz(Christian Suer)



#ifndef TIMELOG_H
#define TIMELOG_H
#pragma once

#include <log.h>
#include <chrono>
#include <string>


// macro shortcuts
#define TIMER_START Timer timer(__FUNCTION__);      // start timer
#define TIMER_ELAPSED_MS timer.elapsed_microsec()   // get elapsed time in ms
#define TIMER_STOP timer.stop();                    // stop timer and log elapsed time (since start or last stop)
#define TIMER_RESTART timer.restart();              // restart timer


class Timer {
public:
    double elapsed_microsec() {
        end = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::duration<double>>(end - begin)).count() / 1000;
    }

    // constructor
    Timer(std::string caller_function = "") : caller_function(caller_function) {
        begin = std::chrono::high_resolution_clock::now();
        if (caller_function == "") {
            Log::info("timer started");
        }
        else {
            Log::info("timer started in scope ", caller_function);
        }
    }

    void stop() {
        double elapsed_since_start = elapsed_microsec();
        if (caller_function == "") {
            Log::info("timer stopped after ", elapsed_since_start, " microsec");
        }
        else {
            Log::info("timer in scope ", caller_function, " stopped after ", elapsed_microsec(), " microsec");
        }
        stopped = true;
        begin = std::chrono::high_resolution_clock::now();
    }

    void restart() {
        begin = std::chrono::high_resolution_clock::now();
        stopped = false;
    }

    // destructor
    ~Timer() {
        double elapsed_since_start = elapsed_microsec();
        if (!stopped) {
            Log::info("end of timer by reaching end of scope ", caller_function, ": ", elapsed_since_start, " microsec");
        }
    }
private:
    std::chrono::high_resolution_clock::time_point begin, end;
    bool stopped = false;
    std::string caller_function = "";
};

#endif