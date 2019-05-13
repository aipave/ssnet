#include <sys/syscall.h>
#include <unistd.h>
#include <functional>
#include <iostream>

#include "CurrentThread.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

using namespace ssnet;
using namespace std;

void print(EventLoop *loop = nullptr) {
    cout << "pid: " << getpid() << ", tid: " << ::syscall(SYS_gettid) << ", loop: " <<
         reinterpret_cast<size_t>(loop) << endl;
}

void init(EventLoop *loop) {
    printf("init(): pid = %d, tid = %d, loop = %p\n",
           getpid(), GetThreadID(), loop);
}

int main() {
    print();
    EventLoop loop;
    loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

    {
        printf("single thread %p:\n", &loop);
        EventLoopThreadPool pool(&loop);
        pool.setThreadNum(0);
        pool.start(init);
        assert(pool.getNextLoop() == &loop);
        assert(pool.getNextLoop() == &loop);
        assert(pool.getNextLoop() == &loop);
    }

    {
        printf("another thread: \n");
        EventLoopThreadPool pool(&loop);
        pool.setThreadNum(3);
        pool.start(init);
        EventLoop *nextLoop = pool.getNextLoop();
        assert(nextLoop != &loop);
        assert(nextLoop != pool.getNextLoop());
        assert(nextLoop != pool.getNextLoop());
        assert(nextLoop == pool.getNextLoop());
    }

    loop.loop();
}