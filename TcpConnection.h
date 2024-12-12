#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接, 通过accept函数拿到connfd
 *
 * => TcpConnecton 设置回调 => channel => poller => Channel的回调操作
 */
class TcpConnection : noncopyalbe, std::enable_shared_from_this(TcpConnection)
{
public:
    TcpConnection(EventLoop * loop,
                  const std::string &name,
                  const InetAddress &localAddr,
                  const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisConnected; }

    // 发送数据
    void send(const void *message, int len);
    // 关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCakkback(const WriteCompleteCallback &cb) { const writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMark_ = cb;
        highWaterMark_ = highWaterMark;
    }
    
    void setCloseCallback(const CloseCallback &cb) { closeCallBack_ = cb; }

    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void handleRead(Timestamp receiveTime);
    void handleWite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *message, size_t len);
    void shutdownInLoop();

    EventLoop *loop_; // 这里绝对不是baseLoop, 因为TcpConnectoin都是在subLoop里面管理的
    const std::string name_;
    std::atmoic_int state_;
    bool reading_;

    // 这里和Acceptor类似 Accpeor->mainLoop TcpConnection->subLoop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;       // 有新链接时的回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallBack_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};