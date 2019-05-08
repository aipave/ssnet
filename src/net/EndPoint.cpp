#include <string.h>
#include <arpa/inet.h>

#include "EndPoint.h"
#include "ISocket.h"

using namespace ssnet;

bool EndPoint::setAddress(struct sockaddr_in6 &addr6, const std::string& ip, uint16_t port) {
    bzero(&addr6, sizeof addr6);
    addr6.sin6_family = AF_INET6;
    if (::inet_pton(AF_INET6, ip.c_str(), &addr6.sin6_addr) <= 0) {
        return false; // Failed to convert IP
    }
    addr6.sin6_port = htobe16(port);
    return true;
}

bool EndPoint::setAddress(struct sockaddr_in &addr, const std::string &ip, uint16_t port) {
    bzero(&addr, sizeof addr);
    addr.sin_family = AF_INET;
    if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        return false; // Failed to convert IP
    }
    addr.sin_port = htobe16(port);
    return true;
}

EndPoint::EndPoint(const std::string &ip, uint16_t port, bool ipv6) {
    if (ipv6) {
        if (!setAddress(_addr6, ip, port )) {
            bzero(&_addr6, sizeof _addr6);
            _addr6.sin6_family = AF_INET6;
            _addr6.sin6_addr = in6addr_any;
            _addr6.sin6_port = htons(port);
        }
    } else {
        if (!setAddress(_addr, ip, port)) {
            bzero(&_addr, sizeof _addr);
            _addr.sin_family = AF_INET;
            _addr.sin_addr.s_addr = INADDR_ANY;
            _addr.sin_port = htons(port);
        }
    }
}

EndPoint::EndPoint(uint16_t port, bool ipv6) {
    if (ipv6) {
        if (!setAddress(_addr6, "::", port)) {// Use "::" for IPv6 any address
            //TODO handle error
        }
    } else {
        if (setAddress(_addr, "0.0.0.0", port)) {
            //TODO handle error
        }
    }
}

std::string EndPoint::toIpString() const {
    char buf[64];
    ISocket::toIpString(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string EndPoint::toIpPortString() const {
    char buf[64];
    ISocket::toIpPortString(buf, sizeof buf, getSockAddr());
    return buf;
}