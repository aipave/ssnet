#include "Logger.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "TcpClient.h"
#include "EndPoint.h"
#include "EventLoop.h"

using namespace ssnet;

class EchoClient {
public:
    EchoClient(EventLoop *loop, const EndPoint &peer) :
            client_(loop, peer),
            _loop(loop) {
        client_.setConnectionCallback(std::bind(&EchoClient::connectionCallback, this, _1, _2));
        client_.setMessageCallback(std::bind(&EchoClient::messageCallback, this, _1, _2));
    }

    void start() {
        client_.connect();
    }

private:
    void connectionCallback(TcpConnectionPtr conn, bool up) {
        LOG(INFO) << "connection is " << (up ? "up" : "down");
        conn->send("hello");
    }

    void messageCallback(TcpConnectionPtr conn, Buffer *msg) {
        LOG(INFO) << "receive msg: " << msg->retrieveAllAsString();
        _loop->quit();
    }

    TcpClient client_;
    EventLoop *_loop;
};

int main() {
    EventLoop loop;
    EndPoint peer(9102);
    EchoClient client(&loop, peer);
    client.start();
    loop.loop();

    return 0;
}