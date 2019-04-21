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

std::string Timestamp::toString() {
    char buf[32];
    int64_t seconds = _usSinceEpoch / kMicroSecondsPerSecond;
    int64_t microseconds = _usSinceEpoch % kMicroSecondsPerSecond;
    // format the timestamp as seconds.microseconds
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64, seconds, microseconds);
    return buf;
}