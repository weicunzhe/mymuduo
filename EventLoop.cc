#include "EventLoop.h"

// 防止一个线程创建多个EventLoop thread_local
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的poller IO复用接口的超时时间
const int kPollTimeMs = 10000;