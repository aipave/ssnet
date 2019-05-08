#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace ssnet;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop) :
        _loopPtr(baseLoop) {
}

EventLoopThreadPool::~EventLoopThreadPool() {
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb) {
    _loopPtr->assertInLoopThread();
    for (int i = 0; i < _threadNumbers; ++i) {
        EventLoopThread *th = new EventLoopThread(cb);
        _threads.push_back(std::unique_ptr<EventLoopThread>(th));
        _loops.push_back(th->startLoop());
    }
    if (_threadNumbers == 0 && cb) {
        cb(_loopPtr);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    _loopPtr->assertInLoopThread();
    return getLoopForHash(_next++);
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    _loopPtr->assertInLoopThread();
    return _loops.empty() ? _loopPtr : _loops[hashCode % _loops.size()];
}