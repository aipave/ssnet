#include <iostream>
#include <cassert>
#include <string>

#include <fcntl.h>
#include <unistd.h>

#include "Buffer.h"
#include "Logger.h"

using namespace ssnet;
using namespace std;

void testReadableBytes() {
    Buffer buf;
    assert(buf.readableBytes() == 0);

    string str(200, 'x');
    buf.append(str.c_str(), str.size());
    assert(buf.readableBytes() == 200);

    buf.retrieve(50);
    assert(buf.readableBytes() == str.size() - 50);

    buf.append(str.c_str(), str.size());
    assert(buf.readableBytes() == str.size() * 2 - 50);
}

void testAppendReadPeek() {
    Buffer buf;
    uint8_t v8 = 233;
    uint16_t v16 = 65530;
    uint32_t v32 = 23123123;
    uint64_t v64 = 21341411231;
    buf.appendUint8(v8);
    buf.appendUint16(v16);
    buf.appendUint32(v32);
    buf.appendUint64(v64);
    assert(buf.peekUint8() == v8);
    assert(buf.readUint8() == v8);
    assert(buf.peekUint16() == v16);
    assert(buf.readUint16() == v16);
    assert(buf.peekUint32() == v32);
    assert(buf.readUint32() == v32);
    assert(buf.peekUint64() == v64);
    assert(buf.readUint64() == v64);
}

void testReadFd() {
    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return;
    }

    int flags = fcntl(fds[0], F_GETFL, 0);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);

    Buffer buffer;
    const std::string data = "This is some data.";
    ssize_t m = write(fds[1], data.c_str(), data.size());
    if (m == -1) {
        LOG(FATAL) << "write";
    }


    ssize_t n = buffer.readFd(fds[0]);

    close(fds[1]);
    close(fds[0]);

    assert(n == static_cast<ssize_t>(data.size()));

    std::string result = buffer.retrieveAllAsString();
    assert(result == data);

    assert(buffer.readableBytes() == 0);
}

void otherTests() {
    {
        Buffer buffer;
        const std::string data = "hello world!";
        buffer.append(data.c_str(), data.size());
        assert(buffer.readableBytes() == data.size());
        assert(buffer.retrieveAllAsString() == data);
        assert(buffer.readableBytes() == 0);
    }

    {
        Buffer buffer;
        uint32_t num1 = 12345;
        int16_t num2 = -5432;
        buffer.appendUint32(num1);
        buffer.appendInt16(num2);
        assert(buffer.readableBytes() == sizeof(uint32_t) + sizeof(int16_t));
        assert(buffer.readUint32() == num1);
        assert(buffer.readInt16() == num2);
    }

}

int main() {
    testReadableBytes();
    testAppendReadPeek();
    testReadFd();
    otherTests();

    LOG(INFO) << "All tests passed!" ;
    return 0;
}
