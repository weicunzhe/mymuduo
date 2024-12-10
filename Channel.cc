#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/epoll.h>

const int kNoneEvent = 0;
const int kReadEvent = EPOLLIN | EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), event_(0), revent_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

/**
 * 当改变channel所表示fd的events事件后, update负责在poller里面更改fd相应的事件epoll_ctl
 * EventLoop => ChannelList Poller
 */
void Channel::update()
{
    // 通过channel所属的EventLoop, 调用poller的相应方法, 注册fd的events事件
    loop_->updateChannel(this);
}

// 在channel所属的EventLoop中, 把当前的channel删除掉
void Channel::remove()
{
    loop_->removeChannel(this);
}

// channel的tie方法什么时候调用过
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 根据poller通知channel发生的具体事件, 由channel负责调用具体的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d", revent_);

    if ((revent_ & EPOLLHUP) && !(revent_ & EPOLLIN))
    {
        if (closeCallBack_)
            closeCallBack_();
    }

    if (revent_ & EPOLLERR)
    {
        if (errorCallBack_)
            errorCallBack_();
    }

    if (revent_ & (EPOLLIN | EPOLLPRI))
    {
        if (readCallBack_)
            readCallBack_(receiveTime);
    }

    if (revent_ & EPOLLOUT)
    {
        if (writeCallBack_)
            writeCallBack_();
    }
}