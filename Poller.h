#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

/**
 *Poller 是 ​事件循环（EventLoop）的核心组件，
 *负责底层 I/O 多路复用的实现（如 epoll、poll 或 select）
 *，用于监听文件描述符（fd）上的事件，并将活跃事件分发给对应的 Channel
 * 处理。它是 Reactor 模式中的 ​事件分发器（Event Demultiplexer）​。
 *
 */
class Channel;
class EventLoop;

// muduo库中多路事件分发器的核心IO多路复用

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 给所有的IO复用保留唯一的接口
    virtual TimeStamp poll(int timeoutMs, ChannelList *activeChanneles) = 0;
    virtual void upateChannel(Channel *Channel) = 0;
    virtual void removeChannel(Channel *Channel) = 0;

    // 判断参数channel是否在当前的Poller当中
    bool hasChannel(Channel *channel) const;
    // EventLoop可以通过该接口获取默认的IO复用具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // map key：scokcetfd, value sockefd 所属的channel 通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义Poller所属的事件循环EventLoop
};
