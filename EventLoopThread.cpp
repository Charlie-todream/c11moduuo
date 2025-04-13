#include "EventLoopThread.h"

#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name) : loop_(nullptr),
                                                                                          exiting_(false),
                                                                                          thread_(std::bind(&EventLoopThread::threadFunc, this), name),
                                                                                          mutex_(),
                                                                                          cond_(),
                                                                                          callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    // 启动底层的新线程
    thread_.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
    }

    return loop;
}

// 下面这个方案，实在单独新的线程里面运行
void EventLoopThread::threadFunc()
{
    EventLoop loop; // 创建一个独立的EventLoop，和上面的线程是以一一对应，once loop thread
    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}