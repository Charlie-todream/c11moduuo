#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;

// 时间循环类，包括两大模块，Channel Poller
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    TimeStamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放入队列,唤醒loop所在的线程,执行cb
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在的线程
    void wakeup();

    // EventLoop的方法 =》 Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CUrrentThread::tid(); }

private:
    void handleRead();
    void doPendingFunctors(); // 执行回调
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_; // 原子操作，通过CAS实现的
    std::atomic_bool quit_;    // 标识当前loop所在的线程id

    const pid_t threadId_;     // 记录当前loop所在的线程id
    TimeStamp pollReturnTime_; // poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // 主要作用，当前mianloop获取一个新用户的Channel，通过轮训算法选择一个subloop，通过该成员唤醒subloop处理产权
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否需要执行回调操作
    std::vector<Functor> pendingFunctors_;    // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                        // 互斥锁，用来保护上面的vector容器的线程安全操作
};
