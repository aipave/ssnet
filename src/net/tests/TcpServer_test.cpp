#include <iostream>
#include <string>

#include "EventLoop.h"
#include "Buffer.h"
#include "TcpServer.h"
#include "EndPoint.h"
#include "TcpConnection.h"

using namespace ssnet;
using namespace std;

class EchoServer {
public:
    EchoServer(EventLoop *loop, const EndPoint &listenAddr) :
            server(loop, listenAddr) {
        server.setConnectionCallback(std::bind(&EchoServer::connect, this, _1, _2));
        server.setMessageCallback(std::bind(&EchoServer::messageCome, this, _1, _2));
        server.setWriteCompleteCallback(std::bind(&EchoServer::sendSuccess, this, _1));
    }

    void start() {
        server.start();
    }

    void connect(TcpConnectionPtr conn, bool up) {
        if (up)
            cout << "connection up";
        else
            cout << "connection down";
    }

    void messageCome(TcpConnectionPtr conn, Buffer *buffer) {
        string msg = buffer->retrieveAllAsString();
        cout << "recieve: " << msg << endl;
        conn->send(msg);
    }

    void sendSuccess(TcpConnectionPtr conn) {
        cout << "send Success" << endl;
    }

private:
    TcpServer server;
};

int main() {
    EventLoop loop;
    EndPoint local(10010);
    EchoServer server(&loop, local);
    server.start();
    loop.loop();

    return 0;
}