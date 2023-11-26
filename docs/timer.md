[[back to main page]](../../README.md)

## Timer

dependencies: `<chrono>`, `<iostream>`;
___
usage:


#### (1.) `#include <timer.h>`, alternatively include as part of `<utilities.h>`

#### (2.) either create an instance of `Timer`, then use any of the methods
####     - `Timer::elapsed_sec()`
####     - `Timer::elapsed_millisec()`
####     - `Timer::elapsed_microsec()`
#### on this instance of `Timer`,
#### 
#### or set the flag `#define TIMELOG` as a preprocessor directive, then use the macro `TIMER` anywhere in the
#### code start a timer; this will print the timer's lifetime on the console as soon as the timer goes out out
#### of scope, i.e. when the function it lives in ends; commenting out the `#define TIMELOG` flag in this case
#### will stop the logging; the `TIMER` macros in this case won't be compiled and therefore have no performance
#### impact;