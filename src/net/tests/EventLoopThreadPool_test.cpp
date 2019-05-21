#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <iostream>
#include <thread>

#include "CurrentThread.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

using namespace ssnet;
using namespace std;

void print(EventLoop* loop = nullptr) {
    cout << "pid: " << getpid() << ", tid: " << ::syscall(SYS_gettid) << ", loop: " << loop << endl;
}

void init(EventLoop* loop) {
    printf("init(): pid = %d, tid = %d, loop = %p\n", getpid(), GetThreadID(), loop);
}

void testSingleThread() {
    printf("single thread:\n");
    EventLoop loop;
    EventLoopThreadPool pool(&loop);
    pool.setThreadNum(0);
    pool.start(init);
    assert(pool.getNextLoop() == &loop);
    assert(pool.getNextLoop() == &loop);
    assert(pool.getNextLoop() == &loop);
}

void testMultiThread() {
    printf("another thread: \n");
    EventLoop loop;
    EventLoopThreadPool pool(&loop);
    pool.setThreadNum(3);
    pool.start(init);
    EventLoop* nextLoop = pool.getNextLoop();
    assert(nextLoop != &loop);
    assert(nextLoop != pool.getNextLoop());
    assert(nextLoop != pool.getNextLoop());
    assert(nextLoop == pool.getNextLoop());
}

int main() {
    print();

    thread(testSingleThread).join();
    thread(testMultiThread).join();
}
