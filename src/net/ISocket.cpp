#include <string.h>
#include <unistd.h>
#include <cassert>

#include "Logger.h"
#include "ISocket.h"

using namespace ssnet;

int ISocket::createTcpSocket(sa_family_t family) {
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG(FATAL) << "SocketApi::createTcpSocket() errno: " << errno;
    }
    return sockfd;
}

struct sockaddr_in6 ISocket::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localAddr;
    bzero(&localAddr, sizeof localAddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localAddr);
    if (::getsockname(sockfd, ISocket::sockAddrCast(&localAddr), &addrlen) < 0) {
        LOG(ERROR) << "ISocket::getLocalAddr() errno: " << errno;
    }
    return localAddr;
}

struct sockaddr_in6 ISocket::getPeerAddr(int sockfd) {
    struct sockaddr_in6 peerAddr;
    bzero(&peerAddr, sizeof peerAddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof peerAddr);
    if (::getpeername(sockfd, ISocket::sockAddrCast(&peerAddr), &addrlen) < 0) {
        LOG(ERROR) << "ISocket::getPeerAddr() errno: " << errno;
    }
    return peerAddr;
}

void ISocket::close(int sockfd) {
    if (::close(sockfd) < 0) {
        LOG(ERROR) << "ISocket::close() errno: " << errno;
    }
}

void ISocket::listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        LOG(FATAL) << "ISocket::listen() errno: " << errno;
    }
}

void ISocket::bind(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG(FATAL) << "ISocket::bind() errno: " << errno;
    }
}

int ISocket::accept(int sockfd, struct sockaddr_in6 *addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
    int connfd = ::accept4(sockfd, sockAddrCast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        LOG(ERROR) << "ISocket::accept() errno: " << errno;
    }
    return connfd;
}


struct sockaddr *ISocket::sockAddrCast(struct sockaddr_in6 *addr) {
    return static_cast<struct sockaddr *>(static_cast<void *>(addr));
}

const struct sockaddr *ISocket::sockAddrCast(const struct sockaddr_in6 *addr) {
    return static_cast<const struct sockaddr *>(static_cast<const void *>(addr));
}

const struct sockaddr_in6 *ISocket::sockAddrIn6Cast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in6 *>(static_cast<const void *>(addr));
}

const struct sockaddr_in *ISocket::sockAddrInCast(const struct sockaddr *addr) {
    return static_cast<const struct sockaddr_in *>(static_cast<const void *>(addr));
}

ssize_t ISocket::write(int fd, const char *data, size_t len) {
    return ::write(fd, data, len);
}

int ISocket::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = sizeof(optval);
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        return errno;
    else
        return optval;
}

ssize_t ISocket::readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ISocket::readv(sockfd, iov, iovcnt);
}

void ISocket::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        LOG(ERROR) << "ISocket::shutdownWrite() errno" << errno;
    }
}

int ISocket::connect(int sockfd, const struct sockaddr *addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

bool ISocket::isSelfConnect(int sockfd) {
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const struct sockaddr_in *laddr4 = reinterpret_cast<struct sockaddr_in *>(&localaddr);
        const struct sockaddr_in *raddr4 = reinterpret_cast<struct sockaddr_in *>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
               && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port
               && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    } else {
        return false;
    }
}

void ISocket::toIpString(char *buf, size_t size, const struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in *addr4 = sockAddrInCast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6 *addr6 = sockAddrIn6Cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void ISocket::toIpPortString(char *buf, size_t size, const struct sockaddr *addr) {
    toIpString(buf, size, addr);
    size_t end = strlen(buf);
    const struct sockaddr_in *addr4 = sockAddrInCast(addr);
    uint16_t port = ntohs(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}