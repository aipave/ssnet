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

const struct sockaddr* EndPoint::getSockAddr() const {
    static struct sockaddr_storage addrStorage;
    if (family() == AF_INET) {
        struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(&addrStorage);
        *addr = _addr;
        return reinterpret_cast<const struct sockaddr*>(addr);
    } else if (family() == AF_INET6) {
        struct sockaddr_in6* addr6 = reinterpret_cast<struct sockaddr_in6*>(&addrStorage);
        *addr6 = _addr6;
        return reinterpret_cast<const struct sockaddr*>(addr6);
    }
    return nullptr;
}

std::string EndPoint::toIpString() const {
    char buf[64];
    ISocket::toIpString(buf, sizeof buf, getSockAddr());
    return buf;
}

std::string EndPoint::toIpPortString() const {
    char buf[64];
    //ISocket::toIpPortString(buf, sizeof buf, getSockAddr());
    const struct sockaddr *addr = getSockAddr();
    if (family() == AF_INET) {
        ISocket::toIpPortString(buf, sizeof(buf), addr); // 192.168.0.1:8080
    } else if (family() == AF_INET6) {
        const struct sockaddr_in6 *addr6 = ISocket::sockAddrIn6Cast(addr);
        uint16_t port = ntohs(addr6->sin6_port);
        char ipBuf[INET6_ADDRSTRLEN] = "";
        // if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr)) { }// IPv6-mapped IPv4 , "::ffff:" prefix e.g. ::ffff:192.168.0.1

        // Regular IPv6 [::1]:8080 <==> [0000:0000:0000:0000:0000:0000:0000:0001]:8080
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, ipBuf, sizeof(ipBuf));
        snprintf(buf, sizeof(buf), "[%s]:%u", ipBuf, port);
    } else {
        snprintf(buf, sizeof buf, "Unknown family %d", addr->sa_family);
    }

    return buf;
}