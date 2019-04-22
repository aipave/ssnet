#pragma once

#include <arpa/inet.h>


namespace ssnet {
class ISocket {
public:
    static int createTcpSocket(sa_family_t family);
    static struct sockaddr_in6 getLocalAddr(int sockfd);
    static struct sockaddr_in6 getPeerAddr(int sockfd);
    static void close(int sockfd);
    static void listen(int sockfd);
    static void bind(int sockfd, const struct sockaddr *addr);
    static int accept(int sockfd, struct sockaddr_in6 *addr);
    static ssize_t write(int fd, const char *data, size_t len);
    static int getSocketError(int sockfd);
    static ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
    static void shutdownWrite(int sockfd);
    static int connect(int sockfd, const struct sockaddr *addr);
    static bool isSelfConnect(int sockfd);
    static void toIpString(char *buf, size_t size, const struct sockaddr *addr);
    static void toIpPortString(char *buf, size_t size, const struct sockaddr *addr);

public:
    static struct sockaddr *sockAddrCast(struct sockaddr_in6 *addr);
    static const struct sockaddr *sockAddrCast(const struct sockaddr_in6 *addr);
    static const struct sockaddr_in6 *sockAddrIn6Cast(const struct sockaddr *addr);
    static const struct sockaddr_in *sockAddrInCast(const struct sockaddr *addr);
};
}