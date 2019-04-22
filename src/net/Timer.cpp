#include <functional>
#include <cassert>
#include <algorithm>
#include <cstdint>
#include <utility>

#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "Logger.h"
#include "Timer.h"
#include "EventLoop.h"

using namespace ssnet;

std::atomic <uint32_t> Timer::s_numCreated{0};

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.usSinceEpoch() - Timestamp::now().usSinceEpoch();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

int createTimerfd() {
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
        LOG(FATAL) << "Failed in timerfd_create";
    return timerfd;
}

void readTimerfd(int timerfd) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany)
        LOG(ERROR) << "TimerQueue::hadleRead() reads " << n << " bytes instead of 8";
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newValue;
    bzero(&newValue, sizeof newValue);
    struct itimerspec oldVaule;
    bzero(&oldVaule, sizeof oldVaule);

    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldVaule);
    if (ret) {
        LOG(FATAL) << "timerfd_settime()";
    }
}

auto TimerCmp = [](Timer *lhs, Timer *rhs) {
    if (lhs->expiration() == rhs->expiration())
        return lhs->getTimerId() < rhs->getTimerId();
    else
        return lhs->expiration() < rhs->expiration();
};

TimerQueue::TimerQueue(EventLoop *loop) :
        _loop(loop),
        _timerFd(createTimerfd()),
        _timerChannel(loop, _timerFd),
        _timers(TimerCmp) {
    _timerChannel.setReadableCallback(std::bind(&TimerQueue::handleRead, this));
    _timerChannel.enableReading();
}

TimerQueue::~TimerQueue() {
    ::close(_timerFd);
    for (auto & timer: _timers) {
        delete timer;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp time, double interval) {
    Timer *timer = new Timer(std::move(cb), time, interval);
    TimerId id = timer->getTimerId();
    _loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return id;
}

void TimerQueue::cancel(TimerId id) {
    _loop->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, id));
}

void TimerQueue::addTimerInLoop(Timer *timer) {
    bool earlistChanged = insertTimer(timer);
    if (earlistChanged)
        resetTimerfd(_timerFd, timer->expiration());
}

void TimerQueue::cancelInLoop(TimerId id) {
    if (_timerTaskWorkingFlag) {
        _cancelingTimers.insert(id);
    } else {
        deleteTimer(id.timer_);
    }
}

bool TimerQueue::insertTimer(Timer *timer) {
    bool earliestChanged;
    if (_timers.empty())
        earliestChanged = true;
    else
        earliestChanged = timer->expiration() < (*_timers.begin())->expiration();
    _timers.insert(timer);
    return earliestChanged;
}

void TimerQueue::deleteTimer(Timer *timer) {
    assert(_timers.erase(timer));
    delete timer;
}

void TimerQueue::handleRead() {
    _loop->assertInLoopThread();
    readTimerfd(_timerFd);

    // 取消定时器
    for (TimerId id: _cancelingTimers)
        deleteTimer(id.timer_);
    _cancelingTimers.clear();

    // 提出过时定时器
    Timestamp now = Timestamp::now();
    std::vector < Timer * > expired = getExpired(now);

    // 完成定时器任务
    _timerTaskWorkingFlag = true;
    for (Timer *timer: expired)
        timer->run();
    _timerTaskWorkingFlag = false;

    // 重置定时器
    reset(expired, now);
}

std::vector<Timer *> TimerQueue::getExpired(Timestamp now) {
    Timer timer([] { }, now, 0);
    auto end = _timers.lower_bound(&timer);
    std::vector < Timer * > expired{_timers.begin(), end};
    _timers.erase(_timers.begin(), end);
    return expired;
}

void TimerQueue::reset(const std::vector<Timer *> &expired, Timestamp now) {
    // 遍历所有定时器,若是重复定时器则加入,否则删除
    for (Timer *timer: expired) {
        if (timer->repeat() && _cancelingTimers.find(timer->getTimerId()) == _cancelingTimers.end()) {
            timer->restart(now);
            insertTimer(timer);
        } else {
            delete timer;
        }
    }

    if (!_timers.empty()) {
        resetTimerfd(_timerFd, (*_timers.begin())->expiration());
    }
}