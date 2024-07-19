#include <sys/time.h>
#include <iostream>
#include <chrono>
#include <string>
#include <random>

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

// 生成一个3000字节的随机字符串
std::string generateRandomString() {
    size_t length = 3000;
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_int_distribution<size_t> distribution{0, characters.size() - 1};

    std::string random_string;
    random_string.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        random_string += characters[distribution(engine)];
    }

    return random_string;
}
