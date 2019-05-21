#include <iostream>

#include "Timer.h"
#include "EventLoop.h"

using namespace ssnet;
using namespace std;

EventLoop *g_loop = nullptr;

void print(const char *msg) {
    static int cnt = 0;
    cout << "msg " << Timestamp::now().toString() << " " << msg << endl;
    if (++cnt == 12 && g_loop) {
        g_loop->quit();
    }
}

void now() {
    cout << "now " << Timestamp::now().toString() << endl;
}

int main() {
    now();
    EventLoop loop;
    g_loop = &loop;

    // delay task
    loop.runAfter(1, [](){ print("once 1.0s"); });
    loop.runAfter(2, [](){ print("once 2.0s"); });
    loop.runAfter(3.6, [](){ print("once 3.5s"); });

    //
    loop.runEvery(4, [](){ print("every 4.0s"); });

    //
    auto id = loop.runEvery(5, [](){ print("every 5.0s"); });
    loop.runAfter(11, [&id, &loop]() { loop.cancel(id); cout << "canceled every 5.0s" << endl; });

    loop.loop();
}