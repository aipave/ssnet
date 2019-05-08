#pragma once

#include <memory>
#include <functional>

#include "NonCopyable.h"
#include "EndPoint.h"

namespace ssnet {
class Channel;

class EventLoop;

class Connector : NonCopyable,
                  public std::enable_shared_from_this<Connector> {
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop *loop, const EndPoint &peer);

    ~Connector();

    void start(); // 线程安全
    void stop(); // 线程安全
    void restart();

    void setNewConnectionCallback(NewConnectionCallback cb) { _onNewConnCb = std::move(cb); }

private:
    enum class NetStatus {
        Disconnected, Connecting, Connected
    };
    static const int kRetryMsDelay = 500;
    static const int kMaxRetryMsDelay = 30 * 1000;

    void startInLoop();

    void stopInLoop();

    void setNetStatus(NetStatus state) { _status = state; }

    void connect();

    void connecting(int sockfd);

    void retry(int sockfd);

    void handleWrite();

    void handleError();


    EventLoop *_loop;
    EndPoint _peer;
    bool _connFlag = false; // 表示是否要链接
    NetStatus _status = NetStatus::Disconnected;
    std::unique_ptr <Channel> _chan; // connect时,channel可读则成功
    NewConnectionCallback _onNewConnCb;
    int _retryMsDelay = kRetryMsDelay;
};
} // namespace ssnet