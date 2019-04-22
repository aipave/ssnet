#include <sys/uio.h>

#include "Buffer.h"
#include "ISocket.h"
#include "Logger.h"

using namespace ssnet;

ssize_t Buffer::readFd(int fd) {
    char extraBuffer[65536];
    const int iovcnt = writableBytes() < sizeof extraBuffer ? 2 : 1;
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = _data + _writerIndex;
    vec[0].iov_len = writable;
    if (iovcnt == 2) {
        vec[1].iov_base = extraBuffer;
        vec[1].iov_len = sizeof extraBuffer;
    }
    const ssize_t n = ISocket::readv(fd, vec, iovcnt);
    if (n < 0) {
        // Handle read error
        auto handleError = [&, this, fd]() {
            LOG(ERROR) << "Buffer::readFd() read error, errno: " << errno;
            // Retry once
            ssize_t n = ISocket::readv(fd, vec, iovcnt);
            if (n >= 0) {
                if (static_cast<size_t>(n) <= writable) {
                    _writerIndex += n;
                } else {
                    _writerIndex = _size;
                    append(extraBuffer, n - writable);
                }
            } else {
                // Failed to read, close
                // TODO notifying user and close
                LOG(ERROR) << "Buffer::readFd() read error on retry, errno: " << errno;
            }
        };

        handleError();
    } else if (static_cast<size_t>(n) <= writable)
        _writerIndex += n;
    else {
        _writerIndex = _size;
        append(extraBuffer, n - writable);
    }
    return n;
}