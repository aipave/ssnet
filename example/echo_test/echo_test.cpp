#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "EndPoint.h"

using namespace ssnet;

class EchoServer {
public:
    EchoServer(EventLoop* loop, const EndPoint& listenAddr) :
            _loop(loop),
            _svr(loop, listenAddr)
    {
        _svr.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2));
    }

    void start() {
        _svr.start();
    }

private:
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
        conn->send(*buf);
    }

    EventLoop* _loop;
    TcpServer _svr;
};

int main() {
    EventLoop loop;
    EndPoint listenAddr(9102);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}