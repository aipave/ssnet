#include <iostream>
#include <cassert>

#include "Timestamp.h"
#include "Logger.h"

void testInvalidTime() {
    ssnet::Timestamp ts = ssnet::Timestamp::invalidTime();
    assert(!ts.valid());
    assert(ts.usSinceEpoch() == 0);
}

void testNow() {
    ssnet::Timestamp ts = ssnet::Timestamp::now();
    assert(ts.valid());
}

void testTimeDiff() {
    ssnet::Timestamp ts1(1000000); // 1s
    ssnet::Timestamp ts2(3000000); // 3s
    double diff = ssnet::Timestamp::timeDiff(ts2, ts1);
    assert(diff == 2.0);
}

void testAddTime() {
    ssnet::Timestamp ts(1000000); // 1s
    ts.addTime(2.5); // Add 2.5s
    assert(ts.usSinceEpoch() == 3500000); // 3.5s
}

int main() {
    // Test invalidTime()
    testInvalidTime();

    // Test now()
    testNow();

    // Test timeDiff()
    testTimeDiff();

    // Test addTime()
    testAddTime();

    LOG(INFO) << "All tests passed!\n";
    return 0;
}
