#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string.h>

#include "Logger.h"
#include "TcpSocket.h"
#include "ISocket.h"

using namespace ssnet;

TcpSocket::~TcpSocket() {
    ISocket::close(_fd);
}

void TcpSocket::listen() {
    ISocket::listen(_fd);
}

void TcpSocket::bind(const EndPoint &local) {
    ISocket::bind(_fd, local.getSockAddr());
}

void TcpSocket::setReusePort(bool enable) {
    int optval = enable ? 1 : 0;
    int ret = ::setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0)
        LOG(ERROR) << "TcpSocket::setReusePort() errno: " << errno;
}

void TcpSocket::setReuseAddr(bool enable) {
    int optval = enable ? 1 : 0;
    int ret = ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0)
        LOG(ERROR) << "TcpSocket::setReuseAddr() errno: " << errno;
}

void TcpSocket::setTcpNoDelay(bool enable) {
    int optval = enable;
    int ret = ::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0)
        LOG(ERROR) << "TcpSocket::setTcpNoDelay() errno: " << errno;
}

int TcpSocket::accept(EndPoint *peer) {
    struct sockaddr_in6 addr;
    bzero(&addr, sizeof addr);
    int connfd = ISocket::accept(_fd, &addr);
    if (connfd >= 0) {
        peer->setSockAddrIn6(addr);
    }
    return connfd;
}

void TcpSocket::shutdownWrite() {
    ISocket::shutdownWrite(_fd);
}