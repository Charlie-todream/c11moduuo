#pragma once

#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include <sys/epoll.h>

class Channel;

/**
 *  epoll的使用
 *   epoll_create
 *   epoll_ctl add/mod/del
 *   epoll_wait
 *   EpollPoller 是 ​基于 epoll 的 I/O 多路复用模块，
 *   负责监听和管理文件描述符（如 socket）上的 I/O 事件（读、写、错误等），
 *   并将事件通知给上层事件循环（EventLoop）
 */

class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;
    // 重写基类Poller的抽象方法
    TimeStamp poll(int timeoutMs, ChannelList *activeChannls) override;
    void upateChannel(Channel *Channel) override;
    void removeChannel(Channel *Channel) override;

private:
    static const int kInitEventListSize = 16;

    // 填写活跃的链接
    void fileActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel通道
    void update(int operation, Channel *channel);
    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};