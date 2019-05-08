#include "Logger.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace ssnet;

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb) :
        _threadInitCb(cb) {
}

EventLoopThread::~EventLoopThread() {
    if (_loop) {
        _loop->quit();
        _thread->join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(_thread == nullptr);
    _thread = std::make_unique<std::thread>(std::bind(&EventLoopThread::threadFunc, this));
    std::unique_lock <std::mutex> lock(_mtx);
    _cv.wait(lock, [&] { return _loop; });
    return _loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (_threadInitCb)
        _threadInitCb(&loop);
    {
        std::lock_guard <std::mutex> lock(_mtx);
        _loop = &loop;
        _cv.notify_one();
    }

    loop.loop();
    std::lock_guard <std::mutex> lock(_mtx);
    _loop = nullptr;
}