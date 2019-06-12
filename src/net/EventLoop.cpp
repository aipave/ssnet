#include <cassert>

#include <unistd.h>
#include <sys/eventfd.h>

#include "Logger.h"
#include "CurrentThread.h"
#include "EventLoop.h"
#include "EPollPoller.h"


using namespace ssnet;

namespace {
thread_local EventLoop
*
t_loopInThisThread = nullptr;
}

int createEventfd() {
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0)
        LOG(FATAL) << "fail in createEventfd()";
    return eventfd;
}


EventLoop::EventLoop() :
        _tid(GetThreadID()),
        _poller(new EPollPoller(this)),
        _timerQueue(this),
        _wakeupFd(createEventfd()),
        _wakeupChan(this, _wakeupFd),
        _mtx() {
    if (t_loopInThisThread) {
        LOG(FATAL) << "another EventLoop exists in this thread" << _tid;
    } else {
        t_loopInThisThread = this;
    }
    _exitFlag = false;
    _wakeupChan.setReadableCallback(std::bind(&EventLoop::handleRead, this));
    _wakeupChan.enableReading();
}

EventLoop::~EventLoop() {
    // ::close(_wakeupFd);
}

bool EventLoop::isInLoopThread() const {
    return _tid == GetThreadID();
}

void EventLoop::loop() {
    assert(!_isLooping);
    assertInLoopThread();
    _isLooping = true;
    while (!_exitFlag) {
        std::vector < Channel * > activeChannels;
        static const int kPollTimeMs = 10000;
        Timestamp pollReturnTime = _poller->poll(kPollTimeMs, activeChannels);
        for (Channel *channel: activeChannels) {
            channel->handleEvent(pollReturnTime);
        }
        doTask();
    }
    _isLooping = false;
}

void EventLoop::quit() {
    _exitFlag = true;
    if (!isInLoopThread())
        wakeup();
}

void EventLoop::runInLoop(TaskCallback task) {
    if (isInLoopThread()) {
        task();
    } else {
        queueInLoop(std::move(task));
    }
}

void EventLoop::queueInLoop(TaskCallback task) {
    {
        std::lock_guard <std::mutex> lock(_mtx);
        _tasks.push_back(std::make_shared<TaskCallback>(std::move(task)));
    }
    if (!isInLoopThread() || _workingFlag) {
        wakeup();
    }
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb) {
    return _timerQueue.addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
    Timestamp time = Timestamp::now();
    time.addTime(delay);
    return _timerQueue.addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
    Timestamp time = Timestamp::now();
    time.addTime(interval);
    return _timerQueue.addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId id) {
    _timerQueue.cancel(id);
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    // LOG(DEBUG) << "EventLoop::updateChannel() fd: " << channel->fd();
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    // LOG(DEBUG) << "EventLoop::removeChannel() fd: " << channel->fd();
    _poller->removeChannel(channel);
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
        LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes";
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
        LOG(ERROR) << "EventLoop::handleRead() reads " << n << " bytes";
}

void EventLoop::doTask() {
    _workingFlag = true;

    decltype(_tasks) tasks;
    {
        std::lock_guard <std::mutex> lock(_mtx);
        tasks.swap(_tasks);
    }

    for (const auto &task: tasks) {
        (*task)();
    }

    _workingFlag = false;
}