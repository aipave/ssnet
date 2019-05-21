#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>

#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace ssnet;
using namespace std;

void print(EventLoop* loop) {
    cout << "pid: " << getpid() << ", tid: " << ::syscall(SYS_gettid) << ", loop: " << loop << endl;
}

void quit(EventLoop* loop) {
    print(loop);
    loop->quit();
}

void testEventLoopThread1() {
    EventLoopThread thread1;
}

void testEventLoopThread2() {
    EventLoopThread thread2;
    EventLoop* loop2 = thread2.startLoop();
    loop2->runInLoop(bind(print, loop2));
    cout << "loop2 " << loop2 << endl;
}

void testEventLoopThread3() {
    EventLoopThread thread3;
    EventLoop* loop3 = thread3.startLoop();
    loop3->runInLoop(bind(quit, loop3));
    cout << "loop3 " << loop3 << endl;
}

int main() {
    EventLoop* mainLoop = new EventLoop();
    print(mainLoop);

    testEventLoopThread1();
    testEventLoopThread2();
    testEventLoopThread3();

    return 0;
}