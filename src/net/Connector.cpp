#include "Logger.h"
#include "ISocket.h"
#include "EventLoop.h"
#include "Connector.h"

using namespace ssnet;

const int Connector::kMaxRetryMsDelay;

Connector::Connector(EventLoop *loop, const EndPoint &peer) :
        _loop(loop),
        _peer(peer) {
    LOG(DEBUG) << "Connector::Connector()";
}

Connector::~Connector() {
    LOG(DEBUG) << "Connector::~Connector()";
    if (_chan) {
        int sockfd = _chan->fd();
        _chan.reset();
        ISocket::close(sockfd); // Close the socket fd
    }
}

void Connector::start() {
    _connFlag = true;
    _loop->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::startInLoop() {
    LOG(DEBUG) << "Connector::startInLoop()";
    _loop->assertInLoopThread();
    assert(_status == NetStatus::Disconnected);
    if (_connFlag)
        connect();
    else
        LOG(DEBUG) << "do no connect";
}

void Connector::connect() {
    int sockfd = ISocket::createTcpSocket(_peer.family());
    int ret = ISocket::connect(sockfd, _peer.getSockAddr());
    int savedErrno = ret == 0 ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG(ERROR) << "connect error in Connector::startInLoop " << savedErrno;
            ISocket::close(sockfd);
            break;

        default:
            LOG(ERROR) << "Unexpected error in Connector::startInLoop " << savedErrno;
            ISocket::close(sockfd);
            // connectErrorCallback_();
            break;
    }
}

void Connector::connecting(int sockfd) {
    LOG(DEBUG) << "Connector::connecting";
    setNetStatus(NetStatus::Connecting);
    _chan = std::make_unique<Channel>(_loop, sockfd, false); // 这里的channel不管理fd资源
    _chan->setWritableCallback(std::bind(&Connector::handleWrite, shared_from_this()));
    _chan->setErrorCallback(std::bind(&Connector::handleError, shared_from_this()));
    _chan->enableWriting();
}

void Connector::handleWrite() {
    if (_status == NetStatus::Connecting) {
        assert(_chan);
        int sockfd = _chan->fd();
        _chan.reset();
        int err = ISocket::getSocketError(sockfd);
        if (err) {
            LOG(ERROR) << "Connector::handleWrite() sock error " << err;
            retry(sockfd);
        } else if (ISocket::isSelfConnect(sockfd)) {
            LOG(WARN) << "Connector::handleWrite() self connect";
            retry(sockfd);
        } else {
            setNetStatus(NetStatus::Connected);
            if (_connFlag)
                _onNewConnCb(sockfd);
            else
                ISocket::close(sockfd);
        }
    }
}

void Connector::handleError() {
    LOG(ERROR) << "Connector::handleError() errno: " << errno;
}

void Connector::retry(int sockfd) {
    ISocket::close(sockfd);
    setNetStatus(NetStatus::Disconnected);
    if (_connFlag) {
        LOG(DEBUG) << "Connector::retry() in " << _retryMsDelay << " milliseconds";
        _loop->runAfter(_retryMsDelay / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        _retryMsDelay = std::min(_retryMsDelay * 2, kMaxRetryMsDelay);
    } else
        LOG(DEBUG) << "do not connect";
}

void Connector::stop() {
    _connFlag = false;
    _loop->queueInLoop(std::bind(&Connector::stopInLoop, shared_from_this()));
}

void Connector::stopInLoop() {
    _loop->assertInLoopThread();
    if (_status == NetStatus::Connecting) {
        setNetStatus(NetStatus::Disconnected);
        if (_chan) {
            int sockfd = _chan->fd();
            _chan.reset();
            ISocket::close(sockfd);
            retry(sockfd);
        }
    }
}

void Connector::restart() {
    _loop->assertInLoopThread();
    setNetStatus(NetStatus::Disconnected);
    _retryMsDelay = kRetryMsDelay;
    _connFlag = true;
    startInLoop();
}
