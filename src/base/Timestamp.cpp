#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "Timestamp.h"

using namespace ssnet;

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

// PRId64 用来兼容不同平台下输出64位整数的不同格式符
std::string Timestamp::toString() {
    char buf[32];
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf) - 1, "%"
    PRId64
    ".%06"
    PRId64, seconds, microseconds);
    return buf;
}