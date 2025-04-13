#pragma once
#include <unistd.h>
#include <sys/syscall.h>
namespace CUrrentThread
{
    // __thread每个线程拥有该变量的独立副本
    extern __thread int t_cachedTid;
    void cacheTid();

    inline int tid()
    {
        // 为编译器提供一个分支预测的提示信息，优化代码路径
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}