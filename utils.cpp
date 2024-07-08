#include <sys/time.h>
#include <iostream>


timeval get_current_timeval() {
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return cur_time;
}
