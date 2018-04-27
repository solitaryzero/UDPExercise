#include "../socket/UdpSocket.h"
#include <random>

class Server{
private:
    UdpSocket* udpSock;         //使用的udp socket
    struct sockaddr_in servaddr;//服务器使用的地址+端口

    UdpSocket* initSocket();    //创建并初始化服务器的socket
    struct Package recvPkg();   //尝试接收一个数据包（来源不限）
    int sendString(struct Package p);   //根据Package p的地址与数据，生成字符串并发送

public:
    void startServer();         //初始化服务器
};