#ifndef TIMER_H
#define TIMER_H

#include <chrono>

using namespace std;

#define TIMEOUT 10

#define current_time chrono::high_resolution_clock::now
#define time_stamp chrono::high_resolution_clock::time_point
#define elapsed_time(end, start) chrono::duration_cast<chrono::milliseconds>(end - start).count()

#endif