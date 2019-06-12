#pragma once

#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>
#include <cassert>

#include "Timestamp.h"
#include "NonCopyable.h"
#include "Poller.h"
#include "Callbacks.h"
#include "Timer.h"

namespace ssnet {
class Channel;

class EventLoop : NonCopyable {
public:
    using TaskCallback = std::function<void()>;

    EventLoop();

    ~EventLoop();

    void loop(); // 必须在创建loop的线程调用
    void quit(); // 线程安全
    void runInLoop(TaskCallback task); // 线程安全
    void queueInLoop(TaskCallback task); // 线程安全


    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);


    TimerId runAt(Timestamp time, TimerCallback cb);

    TimerId runAfter(double delay, TimerCallback cb); // 延迟delay秒
    TimerId runEvery(double interval, TimerCallback cb); // 每隔interval秒
    void cancel(TimerId id);

    void assertInLoopThread() { assert(isInLoopThread()); }

    bool isInLoopThread() const;

private:
    void wakeup(); // 唤醒阻塞在wait的事件循环,以继续执行任务
    void handleRead(); // 处理wakeupfd上的读事件
    void doTask();

    bool _isLooping = false;
    std::atomic<bool> _exitFlag{false};
    bool _workingFlag = false;
    const int _tid;

    std::unique_ptr <Poller> _poller;
    TimerQueue _timerQueue;

    int _wakeupFd;
    Channel _wakeupChan;

    std::mutex _mtx;
    std::vector <std::shared_ptr<TaskCallback> > _tasks;
};
}