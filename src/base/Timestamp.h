#pragma once

#include <string>

namespace ssnet {
// 内部以微秒表示,utc
class Timestamp {
public:
    static const int kMicroSecondsPerSecond = 1000 * 1000;

    Timestamp() = default;

    explicit Timestamp(int64_t microSecondsSinceEpoch) :
            microSecondsSinceEpoch_(microSecondsSinceEpoch) {
    }

    static Timestamp invalidTime() { return Timestamp(); };

    static Timestamp now();

    static double timeDifference(Timestamp high, Timestamp low) {
        // high - low
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    std::string toString();

    // 增加seconds
    void addTime(double seconds) {
        int64_t delta = static_cast<int64_t>(seconds * kMicroSecondsPerSecond);
        microSecondsSinceEpoch_ += delta;
    }


private:
    int64_t microSecondsSinceEpoch_ = 0;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
} // namespace ssnet