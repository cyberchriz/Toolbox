// author: cyberchriz(Christian Suer)



#ifndef TIMELOG_H
#define TIMELOG_H

#include <chrono>
#include <log.h>
#include <string>


// macro shortcuts
#define TIMER_START Timer timer(__FUNCTION__);      // start timer
#define TIMER_ELAPSED_MS timer.elapsed_microsec()   // get elapsed time in ms
#define TIMER_STOP timer.stop();                    // stop timer and log elapsed time (since start or last stop)
#define TIMER_RESTART timer.restart();              // restart timer


class Timer {
public:
    double elapsed_sec() {
        end = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::duration<double>>(end - begin)).count();
    }

    // constructor
    Timer(std::string caller_function = "") : caller_function(caller_function) {
        begin = std::chrono::high_resolution_clock::now();
        if (caller_function == "") {
            Log::force("timer started");
        }
        else {
            Log::force("timer started in scope ", caller_function);
        }
    }

    void stop() {
        double elapsed = elapsed_sec();
        if (elapsed > 60) {
            Log::force("timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
                " stopped after ", elapsed / 60.0, " minutes");
        }
        else if (elapsed > 0.01) {
            Log::force("timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
                " stopped after ", elapsed, " seconds");
        }
        else {
            // convert to milliseconds
            elapsed *= 1000;
            if (elapsed > 0.01) {
                Log::force("timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
                    " stopped after ", elapsed, " msec");
            }
            else {
                // convert to microseconds
                elapsed *= 1000;
                if (elapsed > 0.01) {
                    Log::force("timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
                        " stopped after ", elapsed, " µsec");
                }
                else {
                    // convert to nanoseconds
                    elapsed *= 1000;
                    Log::force("timer in scope ", caller_function == "" ? "<unknown>" : caller_function,
                        " stopped after ", elapsed, " nanosec");
                }
            }
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
        if (!stopped) { stop(); }
    }

private:
    std::chrono::high_resolution_clock::time_point begin, end;
    bool stopped = false;
    std::string caller_function = "";
};

#endif