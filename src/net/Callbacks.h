#pragma once

#include <functional>
#include <memory>

namespace ssnet {
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class TcpConnection;

class Buffer;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// 消息到达
using MessageCallback = std::function<void(const TcpConnectionPtr &conn, Buffer *buf)>;
// 连接建立或关闭
using ConnectionCallback = std::function<void(const TcpConnectionPtr &conn, bool up)>;
// 消息发送完成(到达系统缓冲区)
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &conn)>;
// 定时器回调
using TimerCallback = std::function<void()>;
// 连接关闭回调
using CloseCallback = std::function<void(const TcpConnectionPtr &conn)>;

void defaultConnectionCallback(const TcpConnectionPtr &conn, bool up);

void defaultMessageCallback(const TcpConnectionPtr &, Buffer *);

} // namespace ssnet