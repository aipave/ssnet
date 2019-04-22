#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <set>
#include <utility>
#include <cstdint>
#include <functional>

#include "NonCopyable.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace ssnet {
class Timer;

class TimerId {
    friend struct TimerIdHash;

    friend class TimerQueue;

    friend bool operator==(const TimerId, const TimerId);

    friend bool operator<(const TimerId, const TimerId);

public:
    TimerId(uint32_t seq, Timer *timer) :
            sequence_(seq),
            timer_(timer) { }

private:
    uint32_t sequence_;
    Timer *timer_;
};

bool inline operator==(const TimerId lhs, const TimerId rhs) {
    return lhs.sequence_ == rhs.sequence_;
}

bool inline operator<(const TimerId lhs, const TimerId rhs) {
    return lhs.sequence_ < rhs.sequence_;
}

class TimerIdHash {
public:
    std::size_t operator()(const TimerId &id) const {
        return std::hash<uint32_t>()(id.sequence_);
    }
};

class Timer : NonCopyable {
public:
    Timer(TimerCallback cb, Timestamp expiration, double interval) :
            _cb(std::move(cb)),
            _expireTs(expiration),
            _interval(interval),
            _id(s_numCreated++) {
    }

    TimerId getTimerId() { return TimerId(_id, this); }

    Timestamp expiration() const { return _expireTs; }

    void run() { _cb(); }

    bool repeat() const { return _interval > 0.0; }

    void restart(Timestamp now) {
        if (repeat()) {
            now.addTime(_interval);
            _expireTs = now;
        } else
            _expireTs = Timestamp::invalidTime();
    }

private:
    TimerCallback _cb;
    Timestamp _expireTs;
    const double _interval;
    const uint32_t _id;

    static std::atomic <uint32_t> s_numCreated;
};

class EventLoop;

class TimerQueue : NonCopyable {
public:
    TimerQueue(EventLoop *loop);

    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp time, double interval); // 线程安全
    void cancel(TimerId id); // 线程安全

private:
    using Entry = std::pair<Timestamp, Timer *>;

    void addTimerInLoop(Timer *timer);

    void cancelInLoop(TimerId id);

    void handleRead();

    void deleteTimer(Timer *timer);

    bool insertTimer(Timer *timer);

    std::vector<Timer *> getExpired(Timestamp now);

    void reset(const std::vector<Timer *> &expired, Timestamp now);


    EventLoop *_loop;
    const int _timerFd;
    Channel _timerChannel;
    bool _timerTaskWorkingFlag = false;

    std::unordered_set <TimerId, TimerIdHash> _cancelingTimers;
    std::set<Timer *, std::function < bool(Timer * , Timer * )>> _timers;
};
} // namespace ssnet