#include "TcpServer.h"
#include "Logger.h"

EventLoop * CheckLoopNotNull(EventLoop *loop)
{
    if(loop == nullptr) 
    {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const std::string &nameArg,
                     Option option)
                     :loop_(CheckLoopNotNull(loop)),
                     ipPort_(listenAddr.toIpPort()),
                     name_(nameArg),
                     acceptor_(new Acceptor(loop,listenAddr, option == kReusePort)),
                     threadPool_(new EventLoopThreadPool(loop_,name_)),
                     connectionCallback_(),
                     messageCallback_(),
                     nextConnId_(1)
{
    
}
