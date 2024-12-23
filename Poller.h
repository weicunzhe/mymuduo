#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Channel;

// muduo库中多路事件分发器的核心I/O复用模块
class Poller : noncopyalbe
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller();

    // 给所有I/O复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    
    // 判断参数channel是否在当前Poller当中
    virtual bool hasChannel(Channel *channel) const;

    // EventLoop可以通过该接口获取默认的I/O复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);
protected:

    // map的key：sockfd value：sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义Poller所属的事件循环EventLoop
};