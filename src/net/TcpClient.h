#pragma once

#include <mutex>

#include "NonCopyable.h"
#include "EndPoint.h"
#include "Callbacks.h"

namespace ssnet {
class EventLoop;

class Connector;

class TcpClient : NonCopyable {
public:
    using ConnectorPtr = std::shared_ptr<Connector>;

    TcpClient(EventLoop *loop, const EndPoint &peer);

    ~TcpClient();

    void connect(); // 线程安全
    void disconnect(); // 线程安全
    void stop(); // 线程安全

    bool retry() const { return _retryFlag; }

    void enableRetry() { _retryFlag = true; }

    void setConnectionCallback(ConnectionCallback cb) { _onConnCb = std::move(cb); }

    void setMessageCallback(MessageCallback cb) { _onMsgCb = std::move(cb); }

    void setWriteCompleteCallback(WriteCompleteCallback cb) { _onWriteCb = std::move(cb); }

private:
    void newConnection(int sockfd);

    void removeConnectionByPeer(const TcpConnectionPtr &conn);

    void removeConnection(const TcpConnectionPtr &conn);

    EventLoop *_loop;

    ConnectorPtr _connFlagector;
    bool _retryFlag = false;
    bool _connFlag = true;
    ConnectionCallback _onConnCb;
    MessageCallback _onMsgCb;
    WriteCompleteCallback _onWriteCb;

    mutable std::mutex _mtx;
    TcpConnectionPtr _conn;
};
} // namespace ssnet