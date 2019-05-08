#pragma once

#include <string>

#include "NonCopyable.h"
#include "Channel.h"
#include "EndPoint.h"
#include "Callbacks.h"
#include "Buffer.h"

namespace ssnet {
class EventLoop;

class TcpSocket;

// 表示一条连接,一旦断连,本对象无用
// 如果在有数据未写完时关链接,先发送数据再关闭
// 在socket上的封装
class TcpConnection : NonCopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, int sockfd, const EndPoint &local, const EndPoint &peer);

    ~TcpConnection();

    void setMessageCallback(MessageCallback cb) { _onMsgCb = std::move(cb); }

    void setConnectionCallback(ConnectionCallback cb) { _onConnCb = std::move(cb); }

    void setWriteCompleteCallback(WriteCompleteCallback cb) { _onWriteCb = std::move(cb); }

    void setCloseCallback(CloseCallback cb) { _onCloseCb = std::move(cb); } // 内部使用?

    void forceClose(); // 主动关闭,丢弃未发送数据
    void shutdown(); // 关闭,但先发送完数据
    bool send(const char *data, size_t len); // 返回值为true,表示可尝试发送,false表示无法发送(可能已断开连接)
    bool send(Buffer &buffer);

    bool send(const std::string &msg);

    void connectionEstablished(); // for TcpServer
    void connectionDestroyed();

    EventLoop *getLoop() const { return _loop; }

private:
    enum class NetStatus {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    void setNetStatus(NetStatus newNetStatus) { _status = newNetStatus; }

    void handleRead();

    void handleWrite();

    void handleClose();

    void handleError();

    void forceCloseInLoop();

    void shutdownInLoop();

    void sendInLoop(const std::string &msg);

    void sendInLoop(const char *data, size_t len);

    NetStatus _status = NetStatus::Connecting;
    EventLoop *_loop;
    std::unique_ptr <TcpSocket> _socket;
    Channel _chan;
    MessageCallback _onMsgCb;
    ConnectionCallback _onConnCb;
    WriteCompleteCallback _onWriteCb;
    CloseCallback _onCloseCb;
    Buffer _oBuffer;
    Buffer _iBuffer;
    const EndPoint _localEndPoint;
    const EndPoint _peerEndPoint;
};
} // namespace ssnet