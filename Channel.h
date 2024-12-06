#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;

/**
 * 理清楚 EventLoop、Channel、Poller之间的关系 《= Reactor模型上对应 Demulitplex
 * Channel 理解为通道, 封装了sockfd和其感兴趣的event, 如EPOLLIN EPOLLOUT事件
 * 还绑定了poller返回的具体事件
 */
class Channel : noncopyalbe
{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd得到poller通知以后，处理事件
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallBack(ReadEventCallBack cb) { readCallBack_ = std::move(cb); }
    void setWriteCallBack(EventCallBack cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallBack(EventCallBack cb) { closeCallBack_ = std::move(cb); }
    void setErrorCallBack(EventCallBack cb) { errorCallBack_ = std::move(cb); }

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void> &);

    int fd() { return fd_; }
    int events() { return event_; }
    void set_events(int revt) { revent_ = revt; }

    void enableReading()
    {
        event_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        event_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        event_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        event_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        event_ = kNoneEvent;
        update();
    }

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return event_ == kNoneEvent; }
    bool isWriting() const { return event_ & kWriteEvent; }
    bool isReading() const { return event_ & kReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, Poller监听的对象
    int event_;       // 注册fd感兴趣的事件
    int revent_;      // poller返回的具体发生的事件
    int index_;       // 表示channel 状态

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channel通道里面能够获知fd最终发生的具体的事件revents, 所以它负责调用具体事件的回调操作
    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};