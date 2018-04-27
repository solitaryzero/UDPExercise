#include "../common.h"
  
#include <stdio.h>  
#include <stdlib.h>  
  
#include <string.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <arpa/inet.h>  
#include <vector>
#include <iostream>

struct Message{         //udp数据包
    int sequenceNum;
    int length;
    int randomId;
    char checkSum;
    std::vector<char> data;
};

struct Package{         //带有源信息的数据包
    struct sockaddr_in peerAddr;
    struct Message msg;  
};

union NumberMsg{        //便于将int转换为数据发出
    int num;
    char dat[4];
};

class UdpSocket{        //包装后的udp socket
private:
    int sockfd;                                 //实例对应的fd
    char buffer[MAXBUFSIZE];                    //接收数据缓冲区
    char sendBuffer[MAXBUFSIZE];                //发送数据缓冲区
    socklen_t addrLen = sizeof(struct sockaddr);//sockaddr结构的大小

public:
    UdpSocket();
    UdpSocket(int sockfd);                      //初始化
    int initSocket();                           //设置一些sockopt
    struct Package recvMsg();                   //接收数据
    struct Package recvMsgWithTimeOut();        //以common.h中的MAXWAITTIME为超时时间接收数据
    struct Package recvMsg(struct sockaddr* addr);  //接收数据并将地址信息放在addr中
    int sendMsg(struct Message msg, struct sockaddr* addr); //向addr对应的地址发送Message结构的信息
    int sendMsg(struct Package pkg, struct sockaddr* addr); //向addr对应的地址发送Package结构的信息
    int sendMsg(int seq, int randomId, std::string s, struct sockaddr* addr);   
    //向addr对应的地址发送序列号为seq，随机数为randomId，内容为s的信息
    int shutDownSocket();                       //关闭socket
    char genCheckSum(struct Message msg);       //生成msg对应的校验和
    char genCheckSum(char* buf, int len);       //生成未经包装，长度为len数据包的校验和
    bool examineCheckSum(struct Message msg);   //检测msg的校验和是否合法
    std::string vectorToString(std::vector<char> input);    //将vector<char>转换为std::string
};