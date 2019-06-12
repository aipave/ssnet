#pragma once

#include <functional>

#include "Timestamp.h"
#include "NonCopyable.h"

namespace ssnet {
class EventLoop;

// 可选择性的把fd资源交由chnnel管理
// 处理io事件,fd可能为socket,timerfd
class Channel : NonCopyable {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop *loop, int fd, bool managerResource = true);

    ~Channel();

    EventLoop *ownerLoop() const { return _loop; }

    void setReadableCallback(EventCallback cb) { _onReadCb = std::move(cb); }

    void setWritableCallback(EventCallback cb) { _onWriteCb = std::move(cb); }

    void setCloseCallback(EventCallback cb) { _onCloseCb = std::move(cb); }

    void setErrorCallback(EventCallback cb) { _onErrorCb = std::move(cb); }

    // 必须在loopthread调用s
    void enableReading() {
        _events |= kReadEvent;
        update();
    }

    void disableReading() {
        _events &= ~kReadEvent;
        update();
    }

    void enableWriting() {
        _events |= kWriteEvent;
        update();
    }

    void disableWriting() {
        _events &= ~kWriteEvent;
        update();
    }

    void disableAll() {
        _events = kNoneEvent;
        update();
    }

    void setRevents(int revents) { r_events = revents; } // for poller
    bool getAddToPoll() const { return _pollAddingFlag; } // for poller
    void setAddToPoll(bool added) { _pollAddingFlag = added; } // for poller
    bool isNoEvent() const { return _events == kNoneEvent; }

    int events() const { return _events; }

    int fd() const { return _fd; }
    void resetFdAfterClose(int resetVal) { _fd = resetVal; }

    bool isWriting() const { return _events & kWriteEvent; }

    void handleEvent(Timestamp reciveTime); // for EventLoop

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    // 必须在 loopthread 调用
    void update();

    EventLoop *_loop;
    int _fd;
    bool _mgmtResource;
    int _events{0};
    uint32_t r_events{0};
    bool _pollAddingFlag = false;

    EventCallback _onReadCb;
    EventCallback _onWriteCb;
    EventCallback _onCloseCb;
    EventCallback _onErrorCb;
};
} // namespace ssnet