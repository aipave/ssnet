#include <sys/timerfd.h>
#include <string.h>
#include <functional>
#include <iostream>

#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace ssnet;
using namespace std;

void readTimerfd(int timerfd);

class PeriodicTimer {
public:
    PeriodicTimer(EventLoop *loop, double interval, TimerCallback cb) :
            _loop(loop),
            _interval(interval),
            cb_(cb),
            _timerFd(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
            _timerChannel(loop, _timerFd) {
        assert(_timerFd > 0);
        LOG(DEBUG) << "PeriodicTimer timerfd: " << _timerFd;
        _timerChannel.setReadableCallback(std::bind(&PeriodicTimer::handleRead, this));
        _timerChannel.enableReading();
    }

    void start() {
        struct itimerspec spec;
        bzero(&spec, sizeof spec);
        spec.it_interval = toTimeSpec(_interval);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(_timerFd, 0, &spec, nullptr);
        if (ret < 0) {
            LOG(ERROR) << "timerfd_settime errno: " << errno;
        } else {
            LOG(DEBUG) << "timerfd settime success";
        }
    }

private:

    void handleRead() {
        LOG(DEBUG) << "PeriodicTimer::handleRead";
        readTimerfd(_timerFd);
        if (cb_) cb_();
    }

    static struct timespec toTimeSpec(double seconds) {
        struct timespec ts;
        bzero(&ts, sizeof ts);
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
            nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

    EventLoop *_loop;
    const double _interval;
    TimerCallback cb_;
    int _timerFd;
    Channel _timerChannel;
};

void testPeriodicTimer() {
    EventLoop loop;

    using TimerCallback = function<void()>;

    TimerCallback periodicCallback = []() { cout << "Periodic timer 6 sec" << endl; };

    TimerCallback runEveryCallback = []() { printf("Run every 2 sec\n"); };

    // create and start the periodic timer
    PeriodicTimer timer(&loop, 6, periodicCallback);
    timer.start();

    // schedule a runEvery timer
    loop.runEvery(2, runEveryCallback);

    // start loop
    loop.loop();
}

int main() {
    testPeriodicTimer();
    return 0;
}