#include "../socket/UdpSocket.h"
#include <thread>
#include <random>
#include <map>
#include <deque>
#include <mutex>
#include <signal.h>

struct PackData{
    int seq;
    int retryCount;
    int waitTime;
};

class Client{           //客户端
private:
    UdpSocket* udpSock;                     //使用的udp socket
    struct sockaddr_in servAddr;            //服务器的地址+端口信息
    std::map<int,struct Message> packMap;   //每个sequence number对应的数据包（接收到回复后删除对应条目）
    std::deque<struct PackData> packList;   //待重发的数据包队列（未使用）
    std::vector<char> recvRes;              //收到的回复中数据字段
    std::mutex mtx;                         //互斥锁
    int currentSequenceNum;                 //当前使用的序列号
    int retryCount;                         //当前数据包已重传次数
    bool flag;                              //是否退出程序(输入-1退出)

    UdpSocket* initSocket();                //初始化客户端的socket
    int sendNumber(int num);                //向服务器端发送数字num
    std::vector<char> recvMsg(int seq);     //接收服务器端发来的消息并提取数据字段内容返回

public:
    void startClient();                     //初始化客户端
};