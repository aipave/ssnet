#include <string>
#include <iostream>

#include "EndPoint.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "TcpConnection.h"

using namespace ssnet;
using namespace std;

void onPingMessage(TcpConnectionPtr conn, Buffer *buffer) {
    conn->send(*buffer); // Respond with the same message (Pong)
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cout << "usage: ./pingpongserver ip port threadNum" << endl;
    } else {
        std::string ip(argv[1]);
        uint16_t port = std::stoi(std::string(argv[2]));
        EndPoint listenAddr(ip, port);
        int threadNum = std::stoi(std::string(argv[3]));

        EventLoop loop;
        TcpServer server(&loop, listenAddr);

        server.setMessageCallback(onPingMessage);

        if (threadNum > 1) {
            server.setThreadNum(threadNum);
        }

        server.start();

        loop.loop();
    }
}
