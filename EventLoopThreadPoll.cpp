#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

#include <memory>

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop *baseloop, const std::string &nameArg)

    : baseLoop_(baseloop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPoll::~EventLoopThreadPoll() {};

void EventLoopThreadPoll::start(const ThreadInitCallback &cb)
{
    started_ = true;
    for (int i = 0; i < numThreads_; i++)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop()); // 底层创建线程，绑定一个新的EventLoop，并返回loop的地址
    }

    // 整个服务端只有一个线程，运行这baseloop
    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

// 如果工作在多线程中，baseloop_ 默认以轮询的方式分配channel给subloop
EventLoop *EventLoopThreadPoll::getNextLoop()
{
    EventLoop *loop = baseLoop_;
    if (!loops_.empty()) // 通过轮训的方式获取下一个事件中的loop
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPoll::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}