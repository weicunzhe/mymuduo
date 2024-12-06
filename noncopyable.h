#pragma once

/**
 * noncopyable被继承以后, 派生类对象可以正常的构造和析构, 但是派生类对象
 * 无法进行拷贝构造和赋值操作
 */
class noncopyalbe
{
public:
    noncopyalbe(const noncopyalbe &) = delete;
    noncopyalbe& operator=(const noncopyalbe &) = delete;
protected:
    noncopyalbe() = default;
    ~noncopyalbe() = default;
};