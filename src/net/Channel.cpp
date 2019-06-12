#include <poll.h>
#include <fcntl.h>

#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"
#include "ISocket.h"

using namespace ssnet;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd, bool managerResource) :
        _loop(loop),
        _fd(fd),
        _mgmtResource(managerResource) {
}

Channel::~Channel() {
    if (_pollAddingFlag) {
        _loop->removeChannel(this);
    }
    if (_mgmtResource) {
        _loop->queueInLoop([this]() ->void {
            // close in loop thread
            if (ISocket::close(_fd) == 0) {
                if (fcntl(_fd, F_GETFL) == -1) {
                    resetFdAfterClose(-1);
                }
            }
        });
    }
}

void Channel::handleEvent(Timestamp reciveTime) {
    if ((r_events & POLLHUP) && !(r_events & POLLIN)) {
        if (_onCloseCb) _onCloseCb();
    }

    if (r_events & (POLLERR | POLLNVAL))
        if (_onErrorCb) _onErrorCb();
    if (r_events & (POLLIN | POLLPRI | POLLRDHUP))
        if (_onReadCb) _onReadCb();
    if (r_events & POLLOUT)
        if (_onWriteCb) _onWriteCb();
}

void Channel::update() {
    _loop->updateChannel(this);
}