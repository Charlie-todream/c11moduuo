#pragma once

/**
 *   nonecopyable被继承以后，派生类对象只能正常的构造和析构，
 * 但是派生类不能进行拷贝和赋值操作
 *
 */

class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};