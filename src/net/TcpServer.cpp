#include <cassert>

#include "Logger.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "ISocket.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

using namespace ssnet;

TcpServer::TcpServer(EventLoop *loop, const EndPoint &endpoint, bool reusePort) :
        _loop(loop),
        _acceptor(loop, endpoint, reusePort),
        _threadPool(loop),
        _onConnCb(defaultConnectionCallback),
        _onMsgCb(defaultMessageCallback) {
    assert(loop);
    _acceptor.setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
    LOG(DEBUG) << "tcp server create";
}

TcpServer::~TcpServer() {
    LOG(DEBUG) << "tcp server destruct";
}

void TcpServer::setThreadNum(size_t num) {
    _threadPool.setThreadNum(num);
}

void TcpServer::start() {
    if (_stared) return;
    _stared = true;
    _threadPool.start(_threadInitCb);
    _acceptor.listen();
    LOG(DEBUG) << "tcp server start";
}

void TcpServer::stop() {
    _acceptor.stopListening();
    //for (auto &conn: _connFlags);//conn->close();

    _connFlags.clear();
    LOG(DEBUG) << "tcp server stop";
}

void TcpServer::newConnection(int sockfd, const EndPoint &peer) {
    // 创建connection
    EventLoop *ownerLoop = _threadPool.getNextLoop();
    EndPoint local = ISocket::getLocalAddr(sockfd); // TODO assert local == acceptor.localAddress
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ownerLoop, sockfd, local, peer);
    conn->setMessageCallback(_onMsgCb);
    conn->setConnectionCallback(_onConnCb);
    conn->setWriteCompleteCallback(_onWriteCb);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
    _connFlags.insert(conn);
    ownerLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(TcpConnectionPtr conn) {
    _loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(TcpConnectionPtr conn) {
    _loop->assertInLoopThread();
    assert(_connFlags.erase(conn));
    EventLoop *ownerLoop = conn->getLoop();
    ownerLoop->queueInLoop(std::bind(&TcpConnection::connectionDestroyed, conn));
}