#include <cassert>

#include <fcntl.h>
#include <unistd.h>

#include "Logger.h"
#include "Acceptor.h"
#include "ISocket.h"
#include "TcpConnection.h"
#include "EventLoop.h"

using namespace ssnet;

Acceptor::Acceptor(EventLoop *loop, const EndPoint &endpoint, bool reusePort) :
        _loop(loop),
        _endpoint(endpoint),
        _acceptSocket(ISocket::createTcpSocket(endpoint.family())),
        _acceptChan(loop, _acceptSocket.fd()),
        _idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    _acceptSocket.setReusePort(reusePort);
    _acceptSocket.setReuseAddr(true);
    _acceptSocket.bind(_endpoint);
    _acceptChan.setReadableCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    ::close(_idleFd);
}

void Acceptor::listen() {
    assert(!_listeningFlag);
    _loop->assertInLoopThread();
    _listeningFlag = true;
    _acceptSocket.listen();
    _acceptChan.enableReading();
}

void Acceptor::stopListening() {
    _loop->assertInLoopThread();
    _listeningFlag = false;
    _acceptChan.disableReading();
    ISocket::close(_acceptSocket.fd());
}

void Acceptor::handleRead() {
    _loop->assertInLoopThread();
    EndPoint peer;
    int connfd = _acceptSocket.accept(&peer);
    if (connfd >= 0) {
        if (_onNewConnCb)
            _onNewConnCb(connfd, peer);
        else
            ISocket::close(connfd);
    } else {
        LOG(ERROR) << "Acceptor::handleRead() errno: " << errno;
        if (errno == EMFILE) {
            // 防止cpu100
            ::close(_idleFd);
            _idleFd = ::accept(_acceptSocket.fd(), nullptr, nullptr);
            ::close(_idleFd);
            _idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}