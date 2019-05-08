#pragma once

#include <vector>

#include "Timestamp.h"
#include "NonCopyable.h"

namespace ssnet {
class Channel;

class EventLoop;

class Poller : NonCopyable {
public:
    Poller(EventLoop *loop) : _loop(loop) {
    }

    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, std::vector<Channel *> &channels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

protected:
    EventLoop *_loop;
};
} // namespace ssnet