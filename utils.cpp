#include <sys/time.h>
#include <iostream>
#include <chrono>

timeval get_current_timeval() {
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return cur_time;
}

unsigned long long get_sys_start_timestamp_us() {
    // 获取系统启动到当前时间的纳秒数
    auto uptime = std::chrono::steady_clock::now().time_since_epoch();
    auto uptime_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(uptime).count();

    return uptime_ns / 1000ull;
}