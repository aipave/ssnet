#pragma once

#include <functional>

#include "NonCopyable.h"
#include "EndPoint.h"
#include "Channel.h"
#include "TcpSocket.h"

namespace ssnet {
class EventLoop;

// 内部使用
// 接受新链接
class Acceptor : NonCopyable {
public:
    using NewConnectionCallback = std::function<void(int sockfd, const EndPoint &)>;

    Acceptor(EventLoop *loop, const EndPoint &endpoint, bool reusePort);

    ~Acceptor();

    bool listening() const { return _listeningFlag; }

    void listen(); // loop 线程调用
    void stopListening();

    void setNewConnectionCallback(NewConnectionCallback cb) { _onNewConnCb = std::move(cb); }

private:
    void handleRead();

    bool _listeningFlag = false;
    EventLoop *_loop;
    const EndPoint _endpoint;
    TcpSocket _acceptSocket;
    Channel _acceptChan;
    NewConnectionCallback _onNewConnCb;

    int _idleFd;
};
} // namespace ssnet