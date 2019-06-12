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
        _freeFdSlot(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    _acceptSocket.setReusePort(reusePort);
    _acceptSocket.setReuseAddr(true);
    _acceptSocket.bind(_endpoint);
    _acceptChan.setReadableCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    ::close(_freeFdSlot);
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
    if (ISocket::close(_acceptSocket.fd() == 0)) {
        int flags = fcntl(_acceptSocket.fd(), F_GETFL);
        if (flags == -1) {
            _acceptSocket.resetFdAfterClose(-1);
        }
    }
}

void Acceptor::handleRead() {
    _loop->assertInLoopThread();
    EndPoint peer;
    int connfd = _acceptSocket.accept(&peer);
    if (connfd >= 0) {
        if (_onNewConnCb) {
            _onNewConnCb(connfd, peer);
            return;
        }

        // if _onNewConnCb is not set, close the connection
        if (ISocket::close(connfd) != 0) {
            return;
        }

        // if close is not fail
        if (fcntl(_acceptSocket.fd(), F_GETFL) == -1) {
            _acceptSocket.resetFdAfterClose(-1);
        }
    } else {
        LOG(ERROR) << "Acceptor::handleRead() errno: " << errno;
        if (errno == EMFILE) {
            // 防止cpu100
            ::close(_freeFdSlot);
            _freeFdSlot = ::accept(_acceptSocket.fd(), nullptr, nullptr);
            ::close(_freeFdSlot);
            _freeFdSlot = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}