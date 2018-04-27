#ifndef SERVER_H
#define SERVER_H

#include "../socket/UdpSocket.h"
#include <random>
#include <thread>
#include <map>
#include <deque>
#include <mutex>

struct mySockAddr{
    struct sockaddr_in sin;
    bool operator<(const mySockAddr &o) const {
        return ((sin.sin_addr.s_addr < o.sin.sin_addr.s_addr) || 
                ((sin.sin_addr.s_addr == o.sin.sin_addr.s_addr) &&
                (sin.sin_port < o.sin.sin_port)));
    }
};

class Server{
private:
    UdpSocket* udpSock;         //使用的udp socket
    struct sockaddr_in servaddr;//服务器使用的地址+端口
    std::map<struct mySockAddr, int> queryNum;     //用户的访问次数
    std::deque<std::function<void()>> tasks;             //任务队列
    std::deque<struct Package> packs;               //待发送包队列
    std::mutex taskLock, packLock, queryLock;       //互斥锁

    UdpSocket* initSocket();    //创建并初始化服务器的socket
    struct Package recvPkg();   //尝试接收一个数据包（来源不限）
    int sendString(struct Package p);   //根据Package p的地址与数据，生成字符串并发送
    void genReply(struct Package p);

public:
    void startServer();         //初始化服务器
};

#endif