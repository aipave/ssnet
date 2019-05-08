#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace ssnet {
class EventLoop;

class EventLoopThread;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>; // TODO 参考thread定义

    EventLoopThreadPool(EventLoop *baseLoop);

    ~EventLoopThreadPool();

    // 设置额外的线程数
    void setThreadNum(size_t num) { _threadNumbers = num; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    EventLoop *getLoopForHash(size_t hashCode);

private:
    EventLoop *_loopPtr;
    size_t _threadNumbers = 0;
    size_t _next = 0;
    std::vector<std::unique_ptr<EventLoopThread> > _threads;
    std::vector<EventLoop *> _loops;
};
} // namespace ssnet