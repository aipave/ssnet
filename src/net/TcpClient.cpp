#include <memory>

#include "Logger.h"
#include "Connector.h"
#include "ISocket.h"
#include "EventLoop.h"
#include "TcpClient.h"
#include "TcpConnection.h"

using namespace ssnet;

TcpClient::TcpClient(EventLoop *loop, const EndPoint &peer) :
        _loop(loop),
        _connFlagector(std::make_shared<Connector>(loop, peer)),
        _onConnCb(defaultConnectionCallback) {
    _connFlagector->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, _1));
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique;
    {
        std::lock_guard <std::mutex> lock(_mtx);
        unique = _conn.unique();
        conn = _conn;
    }
    if (conn) {
        CloseCallback cb = std::bind(&TcpClient::removeConnection, this, _1);
        _loop->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique)
            conn->forceClose();
    } else {
        _connFlagector->stop();
    }
}

void TcpClient::connect() {
    _connFlag = true;
    _connFlagector->start();
}

void TcpClient::disconnect() {
    _connFlag = false;
    std::lock_guard <std::mutex> lock(_mtx);
    if (_conn)
        _conn->shutdown();
}

void TcpClient::stop() {
    _connFlag = false;
    _connFlagector->stop();
}

void TcpClient::newConnection(int sockfd) {
    _loop->assertInLoopThread();
    EndPoint local = ISocket::getLocalAddr(sockfd);
    EndPoint peer = ISocket::getPeerAddr(sockfd);
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(_loop, sockfd, local, peer);
    conn->setMessageCallback(_onMsgCb);
    conn->setConnectionCallback(_onConnCb);
    conn->setWriteCompleteCallback(_onWriteCb);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnectionByPeer, this, _1));
    {
        std::lock_guard <std::mutex> lock(_mtx);
        _conn = conn;
    }
    conn->connectionEstablished();
}

void TcpClient::removeConnectionByPeer(const TcpConnectionPtr &conn) {
    {
        std::lock_guard <std::mutex> lock(_mtx);
        assert(conn == _conn);
        _conn.reset();
    }

    _loop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
    if (_retryFlag && _connFlag) {
        LOG(DEBUG) << "TcpClient reconnecting";
        _connFlagector->restart();
    }
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
    _loop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
}