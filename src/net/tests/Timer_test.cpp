#include <iostream>

#include "Timer.h"
#include "EventLoop.h"

using namespace ssnet;
using namespace std;

int cnt = 0;
EventLoop *g_loop;

void print(const char *msg) {
    cout << "msg " << Timestamp::now().toString() << " " << msg << endl;
    if (++cnt == 10 && g_loop)
        g_loop->quit();
}

void now() {
    cout << "now " << Timestamp::now().toString() << endl;
}

int main() {
    now();
    EventLoop loop;
    g_loop = &loop;
    loop.runAfter(1, std::bind(print, "once 1.0"));
    loop.runAfter(2, std::bind(print, "once 2.0"));
    loop.runAfter(3.5, std::bind(print, "once 3.5"));
    TimerId id = loop.runEvery(2, std::bind(print, "every 2.0"));
    loop.runAfter(5, [&id, &loop] {
        loop.cancel(id);
        cout << "canceled every 2.0";
    });
    loop.runEvery(3, std::bind(print, "every 3.0"));
    loop.loop();
}