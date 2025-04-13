#pragma once
#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * EventLoop ，Channel ,Poller
 * Channel 理解为 封装了 socket fd和其感兴趣的event 如 EPLLIN  EPOLLOUT 事件 ,绑定了poller 返回的具体事件
 * ​封装文件描述符（fd）​：每个 Channel 对象管理一个 fd（如 socket、timerfd、eventfd 等）。
 * ​监听事件：注册 fd 上的可读（EPOLLIN）、可写（EPOLLOUT）、错误（EPOLLERR）等事件。
​ * 回调机制：当 fd 发生事件时，调用用户预先注册的回调函数（如读数据、处理连接等）。
 *
 */

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd 得到Poller通知以后，处理事件
    void handleEvent(TimeStamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); } // 把cb左值变为右值
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }   //
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止channel 被手动remove掉，channel还在执行回调操作
    // 将 Channel 与一个外部对象的生命周期绑定，确保在 Channel 执行回调时，该对象不会被意外销毁。
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt)
    {
        revents_ = revt;
    }

    // 设置fd相应的事件状态
    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }

    void disenableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }

    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }

    void disenableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }

    void disenableAll()
    {
        events_ = KNoneEVent;
        update();
    }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == KNoneEVent; }
    bool isWriting() const { return events_ == kWriteEvent; }
    bool isReading() const { return events_ == kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(TimeStamp reveiveTime);
    static const int KNoneEVent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd,Poller 监听的对象
    int events_;      // 注册fd感兴趣的监听事件
    int revents_;     // Poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channel通道里面能够获得通知fd最终发生的具体事件 revenets，所以它负责具体的事件的回调操作

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};