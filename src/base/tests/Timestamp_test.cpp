#include <iostream>
#include <vector>
#include "Timestamp.h"

using namespace std;
using namespace ssnet;

double timeDifference(Timestamp lhs, Timestamp rhs) {
    auto diff = lhs.microSecondsSinceEpoch() - rhs.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

void benchmark() {
    const int kNumber = 1000 * 1000;
    std::vector <Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i)
        stamps.push_back(Timestamp::now());
    cout << "first: " << stamps.front().toString() << endl;
    cout << "last: " << stamps.back().toString() << endl;
    cout << "diff: " << timeDifference(stamps.front(), stamps.back()) << endl;

    const int incLen = 100;
    int increments[incLen] = {0};
    int64_t start = stamps.front().microSecondsSinceEpoch();
    for (int i = 1; i < kNumber; ++i) {
        auto next = stamps[i].microSecondsSinceEpoch();
        auto inc = next - start;
        start = next;
        if (inc < 100) {
            ++increments[inc];
        } else
            cout << "big gap " << inc << endl;
    }

    for (int i = 0; i < incLen; ++i)
        cout << i << " " << increments[i] << endl;
}

int main() {
    cout << Timestamp::now().toString() << endl;
    benchmark();
}