#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include <unordered_set>

#include "NonCopyable.h"
#include "Acceptor.h"
#include "EndPoint.h"
#include "Callbacks.h"
#include "EventLoopThreadPool.h"

namespace ssnet {
class TcpConnection;

class Buffer;

class TcpServer : NonCopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    TcpServer(EventLoop *loop, const EndPoint &endpoint, bool reusePort = true);

    ~TcpServer();

    void setMessageCallback(MessageCallback cb) { _onMsgCb = std::move(cb); }

    void setConnectionCallback(ConnectionCallback cb) { _onConnCb = std::move(cb); }

    void setWriteCompleteCallback(WriteCompleteCallback cb) { _onWriteCb = std::move(cb); }

    // 必须在调用 start 之前设置线程数
    void setThreadNum(size_t num);

    void setThreadInitCallback(const ThreadInitCallback &cb) { _threadInitCb = cb; }

    void start(); // 线程安全
    void stop(); // TODO 线程安全

    void removeConnection(TcpConnectionPtr conn);

private:
    void newConnection(int sockfd, const EndPoint &);

    void removeConnectionInLoop(TcpConnectionPtr conn);

    std::atomic<bool> _stared{false};
    EventLoop *_loop;
    Acceptor _acceptor;
    EventLoopThreadPool _threadPool;
    MessageCallback _onMsgCb;
    ConnectionCallback _onConnCb;
    WriteCompleteCallback _onWriteCb;
    std::unordered_set <TcpConnectionPtr> _connFlags;
    ThreadInitCallback _threadInitCb;
};
} // namespace ssnet