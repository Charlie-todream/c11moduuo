#pragma once

#include "noncopyable.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPoll : noncopyable
{

public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPoll(EventLoop *baseLoop, const std::string &anmeArg);
    ~EventLoopThreadPoll();

    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 如果工作在线程中， baseLoop 默认轮训的方式分配channel 给subloop
    EventLoop *getNextLoop();
    std::vector<EventLoop *> getAllLoops();
    bool stated() const { return started_; }
    const std::string name() const { return name_; }

private:
    EventLoop *baseLoop_;

    std::string name_;
    int started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};