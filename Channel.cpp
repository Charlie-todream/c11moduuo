#include "Channel.h"

#include "Logger.h"
#include "EventLoop.h"
#include <sys/epoll.h>

const int Channel::KNoneEVent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLERR;

const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

// channel的tie方法什么时候调用？ 一个TcpConnection新链接创建的时候 TCPConnection => Channel
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

/**
 * 当改变channel 所表示的fd的events事件后,update负责在poller里面更改fd相应的事件epoll_ctl
 * Eventloop => ChannelList Poller
 */
// TODO
void Channel::update()
{
    // 通过channel所属的EventLoop ，调用poller的相应方法，注册fd的events事件
}

// TODO
void Channel::remove()
{
}

// fd得到poller通知以后，处理事件的
void Channel::handleEvent(TimeStamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知channel发生发生的具体事件，由channel负责的具体的回调操作
void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);

    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    if (revents_ & EPOLLERR)
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}
