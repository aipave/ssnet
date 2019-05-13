#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>

#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace ssnet;
using namespace std;

void print(EventLoop *loop = nullptr) {
    cout << "pid: " << getpid() << ", tid: " << ::syscall(SYS_gettid) << ", loop: " <<
         reinterpret_cast<size_t>(loop) << endl;
}

void quit(EventLoop *loop) {
    print(loop);
    loop->quit();
}

int main() {
    print();
    {
        EventLoopThread th;
    }

    {
        EventLoopThread th;
        EventLoop *loop = th.startLoop();
        loop->runInLoop(std::bind(print, loop));
        cout << "loop2 " << reinterpret_cast<size_t>(loop) << endl;
    }

    {
        EventLoopThread th;
        EventLoop *loop = th.startLoop();
        loop->runInLoop(std::bind(quit, loop));
        cout << "loop3 " << reinterpret_cast<size_t>(loop) << endl;
    }
}