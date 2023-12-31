#pragma once

#include <string>
#include <cstring>

#include <netinet/in.h>

#include "ISocket.h"

namespace ssnet {
class EndPoint {
public:
    EndPoint(const std::string &ip, uint16_t port, bool ipv6 = false);

    explicit EndPoint(uint16_t port = 0, bool ipv6 = false);

    EndPoint(const struct sockaddr_in &addr) {
        ::memmove(&_addr, &addr, sizeof(struct sockaddr_in));
    }

    EndPoint(const struct sockaddr_in6 &addr6) {
        ::memmove(&_addr6, &addr6, sizeof(struct sockaddr_in6));
    }

    const struct sockaddr* getSockAddr() const;

    sa_family_t family() const { return _addr.sin_family; }

    void setSockAddrIn6(const struct sockaddr_in6 addr6) { _addr6 = addr6; }

    std::string toIpString() const;

    std::string toIpPortString() const;


private:
    // Helper function to set IPv4 or IPv6 address and port
    bool setAddress(struct sockaddr_in &addr, const std::string &ip, uint16_t port);
    bool setAddress(struct sockaddr_in6 &addr6, const std::string& ip, uint16_t port);

    union
    {
        struct sockaddr_in _addr;
        struct sockaddr_in6 _addr6;
    };

};
} // namespace ssnet