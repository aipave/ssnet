#pragma once

#include "NonCopyable.h"
#include "EndPoint.h"

namespace ssnet {
class TcpSocket : NonCopyable {
public:
    explicit TcpSocket(int fd) :
            _fd(fd) {
    }

    ~TcpSocket();

    void listen();

    void bind(const EndPoint &);

    void setReusePort(bool enable);

    void setReuseAddr(bool enable);

    void setTcpNoDelay(bool enable);

    int accept(EndPoint *peer);

    void shutdownWrite();

    int fd() const { return _fd; }

private:
    int _fd;
};
} // namespace ssnet