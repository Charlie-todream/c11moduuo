#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <memory>

// 防止一个线程创建多个EventLoop thread_local
__thread EventLoop *t_loopInThisThrad = nullptr;
// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notif唤醒subReactor处理新来的channel
int createEventfd()
{
    // 创建一个事件通知文件描述符  执行时关闭标志（exec时自动关闭）
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false), quit_(false),
      callingPendingFunctors_(false),
      threadId_(CUrrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop create %p in thread %d \n", this, threadId_);
    if (t_loopInThisThrad)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThrad, threadId_);
    }
    else
    {
        t_loopInThisThrad = this;
    }

    //  设置wakeupfd 的事件类型以及发生事件后的回调操作 auto callback = [this]() { this->handleRead(); };
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));

    // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disenableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThrad = nullptr;
}

// 开启事件循环
void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    LOG_INFO("EventLoop %p start looping \n", this);

    while (!quit_)
    {
        activeChannels_.clear();
        // 监听两种类型fd，一种是client的fd，一种是wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            // Poller 监听哪些channel发生事件了，然后上报给EventLoop 通知channel 处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }

        // 执行当前EventLoop 事件循环需要处理的回调操作
        /**
         *
         * IO 线程 mainLoop accept fd <= channel subloop
         * mainLoop 事件注册一个回掉cb（需要subloop来执行） wakeup subloop后，执行下面的方法，执行之前的mainloop注册的cb操作
         *
         */

        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping . \n", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    // 如果是在其他线程中，调用quit，在一个subloop(worker) 中，调用mainLoop（IO）的quit
    if (!isInLoopThread())
    {
        wakeup();
    }
}

// 在当前loop中执行callback
void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) // 在当前的loop线程中，执行cb
    {
        cb();
    }
    else
    { // 在非当前loop线程中执行cb，就需要唤醒loop所在的线程，执行cb
        queueInLoop(cb);
    }
}
// 把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    // 唤醒相应的，需要执行上面回调操作的loop的线程
    // callingPendingFunctots_的意思是 ；在当前loop正在执行回调，但loop又有了新的回调
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8", n);
    }
}

// 用来唤醒loop所在的线程的 向wakeupfd写一个数据，wakeupChannel 就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::wakeup() write %lu bytes instead of 8\n", n);
    }
}

// EventLoop的方法 => Poller的方法
void EventLoop::updateChannel(Channel *channel)
{
    poller_->upateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor &functor : functors)
    {
        functor(); // 执行当前loop需要执行的回调操作
    }

    callingPendingFunctors_ = false;
}