#pragma once

#include <string>

namespace ssnet {

// Internal representation is in microseconds, UTC
class Timestamp {
public:
    static const int kMicroSecondsPerSecond = 1000000;

    Timestamp() = default;

    explicit Timestamp(int64_t usSinceEpoch) :
            _usSinceEpoch(usSinceEpoch) {
    }

    static Timestamp invalidTime() { return Timestamp(); };

    static Timestamp now();

    static double timeDiff(Timestamp high, Timestamp low) {
        // high - low
        int64_t diff = high.usSinceEpoch() - low.usSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    int64_t usSinceEpoch() const { return _usSinceEpoch; }

    bool valid() const { return _usSinceEpoch > 0; }

    std::string toString();

    // add seconds
    void addTime(double seconds) {
        int64_t delta = static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        _usSinceEpoch += delta;
    }


private:
    int64_t _usSinceEpoch = 0;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.usSinceEpoch() < rhs.usSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.usSinceEpoch() == rhs.usSinceEpoch();
}
} // namespace ssnet