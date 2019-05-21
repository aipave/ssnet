#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"

using namespace ssnet;
using namespace std;

void printRunInLoop() {
    cout << "Run in loop" << endl;
}

void printAfterZeroSeconds() {
    cout << "After 0 seconds" << endl;
}

void printEveryThreeSeconds() {
    cout << "Every 3 seconds" << endl;
}

int main() {
    EventLoop loop;

    loop.runInLoop(printRunInLoop);

    loop.runAfter(0, printAfterZeroSeconds);

    loop.runEvery(3, printEveryThreeSeconds);

    // quit the loop after 3 seconds
    loop.runAfter(10, [&loop]() { loop.quit(); });

    loop.loop();

    return 0;
}
