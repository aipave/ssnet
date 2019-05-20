#include <iostream>
#include <cassert>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "Logger.h"
#include "EndPoint.h"

using namespace ssnet;

bool sockaddr6Equal(const sockaddr_in6& addr1, const sockaddr_in6& addr2) {
    return memcmp(&addr1, &addr2, sizeof(sockaddr_in6)) == 0;
}

void testEndPointIPv4() {
    EndPoint ep("127.0.0.1", 8080);
    assert(ep.family() == AF_INET);
    assert(ep.toIpString() == "127.0.0.1");
    assert(ep.toIpPortString() == "127.0.0.1:8080");
}

void testEndPointIPv6() {
    EndPoint ep("::1", 8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "::1");
    assert(ep.toIpPortString() == "[::1]:8080");

    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG(ERROR) << "socket error";
        return;
    }

    struct sockaddr_in6 addr;
    bzero(&addr, sizeof addr);
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(8080);
    addr.sin6_addr = in6addr_any;

    //struct sockaddr* addrCmp = ep.getSockAddr();

    //assert(sockaddr6Equal(*reinterpret_cast<sockaddr_in6*>(addrCmp), addr));

    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof addr) < 0) {
        LOG(ERROR) << "bind error" << std::endl;
        close(sockfd);
        return;
    }

    if (listen(sockfd, 5) < 0) {
        LOG(ERROR) << "listen error" << std::endl;
        close(sockfd);
        return;
    }

    LOG(INFO) << "Listening on [::]:8080" << std::endl;

    // TODO Accept

    close(sockfd);

    ///> client
    //if (::inet_pton(AF_INET6, "::1", &addr.sin6_addr) <= 0) {
    //    LOG(ERROR) << "inet_pton error";
    //    close(sockfd);
    //    return ;
    //}

    //if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof addr) < 0) {
    //    LOG(ERROR) << "connect fail";
    //    close(sockfd);
    //    return;
    //}

    //LOG(INFO) << "Connected to [::1]:8080" << std::endl;

    //close(sockfd);
}

void testEndPointIPv6_2() {
    // Test with an IPv6 address with multiple groups
    EndPoint ep("2001:0db8:85a3:0000:0000:8a2e:0370:7334", 8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "2001:db8:85a3::8a2e:370:7334");
    assert(ep.toIpPortString() == "[2001:db8:85a3::8a2e:370:7334]:8080");
}

void testEndPointIPv6_3() {
    // Test with an IPv6 address using compressed notation
    EndPoint ep("2001:db8::1", 8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "2001:db8::1");
    assert(ep.toIpPortString() == "[2001:db8::1]:8080");
}

void testEndPointIPv6_4() {
    // Test with an IPv6 address using an IPv4-embedded representation
    EndPoint ep("::ffff:192.0.2.1", 8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "::ffff:192.0.2.1");
    assert(ep.toIpPortString() == "[::ffff:192.0.2.1]:8080");

    //int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    //if (sockfd < 0) {
    //    return;
    //}

    //struct sockaddr_in6 serverAddr;
    //bzero(&serverAddr, sizeof serverAddr);
    //serverAddr.sin6_family = AF_INET6;
    //serverAddr.sin6_port = htons(8080);
    //inet_pton(AF_INET6, "::ffff:192.0.2.1", &serverAddr.sin6_addr);

    //if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof serverAddr) < 0) {
    //    LOG(ERROR) << "connect error" << std::endl;
    //    close(sockfd);
    //    return;
    //}

    //LOG(INFO) << "Connected to [::ffff:192.0.2.1]:8080" << std::endl;

    //// Do sth.

    //close(sockfd);

}

void testEndPointAnyIPv4() {
    EndPoint ep(8080);
    assert(ep.family() == AF_INET);
    assert(ep.toIpString() == "0.0.0.0");
    assert(ep.toIpPortString() == "0.0.0.0:8080");
}

void testEndPointAnyIPv6() {
    EndPoint ep(8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "::");
    assert(ep.toIpPortString() == "[::]:8080");
}

void testEndPointInvalidIPv4() {
    EndPoint ep("invalid_ip", 8080);
    assert(ep.family() == AF_INET);
    assert(ep.toIpString() == "0.0.0.0");
    assert(ep.toIpPortString() == "0.0.0.0:8080");
}

void testEndPointInvalidIPv6() {
    EndPoint ep("invalid_ip", 8080, true);
    assert(ep.family() == AF_INET6);
    assert(ep.toIpString() == "::");
    assert(ep.toIpPortString() == "[::]:8080");
}

int main() {
    testEndPointIPv4();
    testEndPointIPv6();
    testEndPointIPv6_2();
    testEndPointIPv6_3();
    testEndPointIPv6_4();
    testEndPointAnyIPv4();
    testEndPointAnyIPv6();
    testEndPointInvalidIPv4();
    testEndPointInvalidIPv6();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
