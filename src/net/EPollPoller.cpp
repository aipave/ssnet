#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cassert>
#include <string.h>
#include <errno.h>

#include "Logger.h"
#include "EventLoop.h"
#include "EPollPoller.h"
#include "Channel.h"

using namespace ssnet;

int createEPoll() {
    int epollfd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollfd < 0)
        LOG(FATAL) << "EPollPoller epoll_create()";
    return epollfd;
}

EPollPoller::EPollPoller(EventLoop *loop) :
        Poller(loop),
        _epollFd(createEPoll()),
        _events(kInitEventListSize) {
}

EPollPoller::~EPollPoller() {
    ::close(_epollFd);
}

Timestamp EPollPoller::poll(int timeoutMs, std::vector<Channel *> &channels) {
    int numEvents = ::epoll_wait(_epollFd, &*_events.begin(), _events.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG(DEBUG) << numEvents << " events happened";
        for (int i = 0; i < numEvents; ++i) {
            Channel *channel = static_cast<Channel *>(_events[i].data.ptr);
            channel->setRevents(_events[i].events);
            channels.push_back(channel);
        }
        if (static_cast<size_t>(numEvents) == _events.size()) {
            _events.resize(_events.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG(DEBUG) << "poll wait nothing";
    } else {
        LOG(ERROR) << "poll wait errno: " << errno;
    }

    // shrink when vector too large and events are in low active
    if (_events.size() > kInitEventListSize && static_cast<size_t>(numEvents) * 2 < _events.size()) {
        _events.resize(_events.size() / 2);
    }

    return now;
}

void EPollPoller::updateChannel(Channel *channel) {
    _loop->assertInLoopThread();
    if (channel->getAddToPoll()) {
        if (channel->isNoEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setAddToPoll(false);
        } else
            update(EPOLL_CTL_MOD, channel);
    } else {
        assert(!channel->isNoEvent());
        update(EPOLL_CTL_ADD, channel);
        channel->setAddToPoll(true);
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    _loop->assertInLoopThread();
    if (channel->getAddToPoll()) {
        update(EPOLL_CTL_DEL, channel);
        channel->setAddToPoll(false);
    }
}

void EPollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    if (::epoll_ctl(_epollFd, operation, channel->fd(), &event) < 0)
        LOG(ERROR) << "epoll_ctl op = " << operation << " fd = " << channel->fd()
                   << " errno: " << errno;
}