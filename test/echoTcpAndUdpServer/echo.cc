#include "muduo/base/Logging.h"
#include "muduo/net/Socket.h"
#include "muduo/net/SocketsOps.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/Channel.h"
#include "muduo/net/TcpServer.h"



using namespace muduo;
using namespace muduo::net;

int createNonblockingUDP()
{
    int sockfd = ::socket(PF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);

    if (sockfd < 0)
        LOG_SYSFATAL << "::socket";

    return sockfd;
}

void serverReadCallback(int sockfd, muduo::Timestamp receiveTime)
{
    char message[512] = {0};
    char addrStr[64];
    struct sockaddr peerAddr;
    memZero(&peerAddr, sizeof(peerAddr));
    socklen_t addrLen = sizeof(peerAddr);
    ssize_t nr = ::recvfrom(sockfd, message, sizeof(message), 0, &peerAddr, &addrLen);

    sockets::toIpPort(addrStr, sizeof(addrStr), &peerAddr);
    LOG_DEBUG << "received" << nr << "bytes from" << addrStr;

    if (nr < 0)
        LOG_SYSERR << "::recvfrom";
    else {
        ssize_t nw = ::sendto(sockfd, message, nr, 0, &peerAddr, addrLen);
        if (nw < 0)
            LOG_SYSERR << "::sendto";
    }
}

void serverConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->name() << " " << conn->peerAddress().toIpPort() << " -> "
          << conn->localAddress().toIpPort() << " is "
          << (conn->connected() ? "UP" : "DOWN");
}

void serverMessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    string msg(buf->retrieveAllAsString());
    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
    conn->send(msg);
}


int main(int argc, char *argv[])
{
    LOG_INFO << "RUNNING ECHO SERVER, TCP PORT: 2007, UDP PORT: 2008";

    Socket sock(createNonblockingUDP());
    sock.bindAddress(InetAddress(2008));
    EventLoop loop;

    Channel channel(&loop, sock.fd());
    channel.setReadCallback(std::bind(&serverReadCallback, sock.fd(), _1));
    channel.enableReading();

    TcpServer server(&loop, InetAddress(2007), "EchoServer");
    server.setConnectionCallback(serverConnectionCallback);
    server.setMessageCallback(serverMessageCallback);
    server.start();
 
    loop.loop();
}
