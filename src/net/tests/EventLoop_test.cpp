#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"

using namespace ssnet;
using namespace std;

int main() {
    EventLoop loop;
    loop.runInLoop([]() { cout << "run in loop" << endl; });
    Timestamp now(Timestamp::now());
    now.addTime(3);
    // loop.runAt(now, [](){cout << "after 3 seconds" << endl;});
    loop.runAfter(0, []() { cout << "after 0 seconds" << endl; });
    loop.runEvery(3, []() { cout << "every 3 seconds" << endl; });
    loop.runAfter(3, [&loop]() { loop.quit(); });
    loop.loop();

    return 0;
}