#include <cassert>

#include "Logger.h"
#include "ISocket.h"
#include "TcpSocket.h"
#include "TcpConnection.h"
#include "EventLoop.h"

using namespace ssnet;


namespace ssnet {
void defaultConnectionCallback(const TcpConnectionPtr &conn, bool up) {
    LOG(DEBUG) << "connection " << (up ? "up" : "down");
}

void defaultMessageCallback(const TcpConnectionPtr &conn, Buffer *buffer) {
    buffer->retrieveAll();
}
} // namespace ssnet



TcpConnection::TcpConnection(EventLoop *loop, int sockfd, const EndPoint &local, const EndPoint &peer) :
        _loop(loop),
        _socket(std::make_unique<TcpSocket>(sockfd)),
        _chan(loop, sockfd),
        _localEndPoint(local),
        _peerEndPoint(peer) {
    _socket->setTcpNoDelay(true);
    _chan.setReadableCallback(std::bind(&TcpConnection::handleRead, this));
    _chan.setWritableCallback(std::bind(&TcpConnection::handleWrite, this));
    _chan.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _chan.setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG(DEBUG) << "TcpConnection::ctor fd: " << _chan.fd();
}

TcpConnection::~TcpConnection() {
    LOG(DEBUG) << "TcpConnction::dtor fd: " << _chan.fd();
    // assert(_status == NetStatus::Disconnected);
}

void TcpConnection::forceClose() {
    if (_status == NetStatus::Connected || _status == NetStatus::Disconnecting) // TODO 非线程安全
    {
        setNetStatus(NetStatus::Disconnecting);
        _loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    _loop->assertInLoopThread();
    if (_status == NetStatus::Connected || _status == NetStatus::Disconnecting) {
        handleClose();
    }
}

void TcpConnection::shutdown() {
    if (_status == NetStatus::Connected) {
        setNetStatus(NetStatus::Disconnecting);
        _loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
    }
}

void TcpConnection::shutdownInLoop() {
    _loop->assertInLoopThread();
    if (!_chan.isWriting()) {
        _socket->shutdownWrite();
    }
}

bool TcpConnection::send(const char *data, size_t len) {
    if (_status == NetStatus::Connected) {
        if (_loop->isInLoopThread())
            sendInLoop(data, len);
        else {
            void (TcpConnection::*fp)(const std::string &) = &TcpConnection::sendInLoop;
            _loop->runInLoop(std::bind(fp, this, std::string(data, len)));
        }
        return true;
    }
    return false;
}

bool TcpConnection::send(Buffer &buffer) {
    if (send(buffer.peek(), buffer.readableBytes())) {
        buffer.retrieveAll();
        return true;
    }
    return false;
}

bool TcpConnection::send(const std::string &msg) {
    return send(msg.data(), msg.size());
}

void TcpConnection::sendInLoop(const std::string &msg) {
    sendInLoop(msg.data(), msg.size());
}

void TcpConnection::sendInLoop(const char *data, size_t len) {
    _loop->assertInLoopThread();
    if (_status == NetStatus::Disconnected) {
        LOG(WARN) << "disconnected, give up writing";
        return;
    }

    // 如果输出队列为空,则尝试直接发送
    ssize_t nwrote = 0;
    size_t remaining = len;
    // bool fatalError = false;
    if (!_chan.isWriting() && _oBuffer.readableBytes() == 0) {
        nwrote = ISocket::write(_chan.fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && _onWriteCb)
                _loop->queueInLoop(std::bind(_onWriteCb, shared_from_this()));
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG(ERROR) << "TcpConnection::sendInLoop()";
                // TODO cp. muduo的fatalerror可能出现的错误
            }
        }
    }

    // 将剩余未发送 buffer 先缓存起来
    assert(remaining <= len);
    if (remaining > 0) {
        _oBuffer.append(data + nwrote, remaining);
        if (!_chan.isWriting())
            _chan.enableWriting();
    }
}

void TcpConnection::connectionEstablished() {
    _loop->assertInLoopThread();
    assert(_status == NetStatus::Connecting);
    setNetStatus(NetStatus::Connected);
    _chan.enableReading();
    _onConnCb(shared_from_this(), true);
}

void TcpConnection::connectionDestroyed() {
    _loop->assertInLoopThread();
    if (_status == NetStatus::Connected) {
        _status = NetStatus::Disconnected;
        _chan.disableAll();

        _onConnCb(shared_from_this(), false);
    }

    //TODO _connectionManager->removeConnection(shared_from_this());
    //TODO _notifyHandler->handleConnectionClosed(shared_from_this());// other conns
}

void TcpConnection::handleRead() {
    _loop->assertInLoopThread();
    ssize_t n = _iBuffer.readFd(_chan.fd()); // 与write不同, read是buffer的成员 —— 处理date from remote peer
    if (n > 0 && _onMsgCb) {
        _onMsgCb(shared_from_this(), &_iBuffer);
    } else if (n == 0) {
        handleClose(); // 收到消息长度0表示关闭
    } else {
        LOG(ERROR) << "TcpConnection::handleRead() errno: " << errno;
    }

}

void TcpConnection::handleWrite() {
    _loop->assertInLoopThread();
    if (_chan.isWriting()) {
        ssize_t n = ISocket::write(_chan.fd(), _oBuffer.peek(), _oBuffer.readableBytes());
        if (n > 0) {
            _oBuffer.retrieve(n);
            if (_oBuffer.readableBytes() == 0) {
                _chan.disableWriting();
                if (_onWriteCb) {
                    _loop->queueInLoop(std::bind(_onWriteCb, shared_from_this()));
                }
                if (_status == NetStatus::Disconnecting) {
                    //shutdownInLoop();
                }
            }
        } else {
            LOG(ERROR) << "TcpConnection::handleWrite() errno: " << errno;
        }
    } else {
        LOG(INFO) << "connection is no more writing()";
    }
}

void TcpConnection::handleClose() {
    _loop->assertInLoopThread();
    assert(_status == NetStatus::Connected || _status == NetStatus::Disconnecting);
    setNetStatus(NetStatus::Disconnected);
    _chan.disableAll();

    _onConnCb(shared_from_this(), false);
    _onCloseCb(shared_from_this());
}

void TcpConnection::handleError() {
    _loop->assertInLoopThread();
    int error = ISocket::getSocketError(_chan.fd());
    LOG(ERROR) << "TcpConnecting::handleError() errno: " << error;
}