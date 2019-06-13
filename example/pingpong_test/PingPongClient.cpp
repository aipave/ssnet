#include <unistd.h>
#include <iostream>
#include <memory>
#include <string>
#include <atomic>
#include <vector>

#include "Logger.h"
#include "CurrentThread.h"
#include "TcpClient.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "EndPoint.h"
#include "TcpConnection.h"

using namespace std;
using namespace ssnet;

class PingPongClient;

class PingPongSession {
public:
    PingPongSession(PingPongClient *owner,
                    EventLoop *loop,
                    const EndPoint &serverAddr) :
            _owner(owner),
            _client(loop, serverAddr) {
        _client.setConnectionCallback(std::bind(&PingPongSession::onConnection, this, _1, _2));
        _client.setMessageCallback(std::bind(&PingPongSession::onPingMessage, this, _1, _2));
    }

    void start() {
        _client.connect();
    }

    void stop() {
        _client.disconnect();
    }

    uint64_t bytesRead() const { return _bytesRead; }

    uint64_t messagesRead() const { return _messagesRead; }

private:
    void onConnection(const TcpConnectionPtr &conn, bool up);

    void onPingMessage(const TcpConnectionPtr conn, Buffer *buffer) {
        ++_messagesRead;
        _bytesRead += buffer->readableBytes();
        _bytesWritten += buffer->readableBytes();
        conn->send(*buffer); // Send the received message back (Ping-Pong)
    }

    PingPongClient *_owner;
    TcpClient _client;
    uint64_t _bytesRead = 0;
    uint64_t _bytesWritten = 0;
    uint64_t _messagesRead = 0;
};

class PingPongClient {
public:
    PingPongClient(EventLoop *loop,
                   EndPoint serverAddr,
                   int threadCount,
                   int messageSize,
                   int sessionCount,
                   int timeout) :
            _loop(loop),
            _threadPool(loop),
            _timeout(timeout),
            _sessionCount(sessionCount) {
        _loop->runAfter(_timeout, std::bind(&PingPongClient::handleTimeout, this));
        if (threadCount > 1)
            _threadPool.setThreadNum(threadCount);
        _threadPool.start();

        for (int i = 0; i < messageSize; ++i)
            _message.push_back(static_cast<char>(i % 128));

        LOG(DEBUG) << "message size:" << _message.size();

        for (int i = 0; i < sessionCount; ++i) {
            auto session = std::make_unique<PingPongSession>(this, _threadPool.getNextLoop(), serverAddr);
            session->start();
            _sessions.emplace_back(std::move(session));
        }
    }

    void start() {
        for (auto &session : _sessions)
            session->start();
    }

    void onConnect() {
        if (++_numConnected == _sessionCount)
            LOG(INFO) << "all clients connected";
    }

    void onDisconnect(const TcpConnectionPtr conn) {
        if (--_numConnected == 0) {
            LOG(INFO) << "all clients disconnected";
            uint64_t totalBytesRead = 0;
            uint64_t totalMessagesRead = 0;
            for (const auto &session : _sessions) {
                totalBytesRead += session->bytesRead();
                totalMessagesRead += session->messagesRead();
            }
            LOG(INFO) << totalBytesRead << " total bytes read";
            LOG(INFO) << totalMessagesRead << " total messages read";
            LOG(INFO) << static_cast<double>(totalBytesRead) / (_timeout * 1024 * 1024) << " MiB/s throughput";
            conn->getLoop()->queueInLoop(std::bind(&PingPongClient::quit, this));
        }
    }

    const string &getMessage() const {
        LOG(DEBUG) << "get message size:" << _message.size();
        return _message;
    }

private:
    void quit() {
        _loop->queueInLoop(std::bind(&EventLoop::quit, _loop));
    }

    void handleTimeout() {
        LOG(INFO) << "timeout";
        for (auto &session : _sessions)
            session->stop();
    }

    EventLoop *_loop;
    EventLoopThreadPool _threadPool;
    int _timeout;
    string _message;
    ::uint64_t _sessionCount;
    std::vector<std::unique_ptr<PingPongSession>> _sessions;
    std::atomic<::uint64_t> _numConnected{0};
};

void PingPongSession::onConnection(const TcpConnectionPtr &conn, bool up) {
    if (up) {
        conn->send(_owner->getMessage());
        _owner->onConnect();
    } else {
        _owner->onDisconnect(conn);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        cout << "usage: ./PongClient server_ip port threads messageSize numSessions time" << endl;
    } else {
        LOG(INFO) << "pid = " << getpid() << ", tid = " << GetThreadID();
        const char *ip = argv[1];
        uint16_t port = stoi(string(argv[2]));
        int threadCount = stoi(string(argv[3]));
        int messageSize = stoi(string(argv[4]));
        int sessionCount = stoi(string(argv[5]));
        int timeout = atoi(argv[6]);

        EventLoop loop;
        EndPoint serverAddr(ip, port);
        PingPongClient client(&loop, serverAddr, threadCount, messageSize, sessionCount, timeout);
        client.start();
        loop.loop();
    }
}