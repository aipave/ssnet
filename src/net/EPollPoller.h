#pragma once

#include <vector>

#include "Poller.h"

struct epoll_event;

namespace ssnet {
class Channel;

class EventLoop;

class EPollPoller : public Poller {
public:
    EPollPoller(EventLoop *loop);

    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, std::vector<Channel *> &channels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    void update(int operation, Channel *channel);

    using EventList = std::vector<struct epoll_event>;

    int _epollFd;
    EventList _events;
};
} // namespace ssnet
