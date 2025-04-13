#include "Poller.h"
#include "EPollPoller.h"
#include <stdlib.h>

Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MuDUO_USE_POLL"))
    {
        return nullptr; // 生成pool实例
    }
    else
    {
        return new EPollPoller(loop);
    }
}