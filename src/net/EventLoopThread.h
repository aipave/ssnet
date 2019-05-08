#pragma once

#include <functional>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "NonCopyable.h"

namespace ssnet {
class EventLoop;

class EventLoopThread : NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback());

    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop *_loop = nullptr;
    std::unique_ptr <std::thread> _thread;
    std::mutex _mtx;
    std::condition_variable _cv;
    ThreadInitCallback _threadInitCb;
};
} // namespace ssnet