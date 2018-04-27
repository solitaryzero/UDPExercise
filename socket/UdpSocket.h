#include "../common.h"
  
#include <stdio.h>  
#include <stdlib.h>  
  
#include <string.h>  
#include <sys/socket.h>  
#include <sys/types.h>  
#include <arpa/inet.h>  
#include <vector>
#include <iostream>

struct Message{
    int sequenceNum;
    int length;
    int randomId;
    char checkSum;
    std::vector<char> data;
};

struct Package{
    struct sockaddr_in peerAddr;
    struct Message msg;  
};

union NumberMsg{
    int num;
    char dat[4];
};

class UdpSocket{
private:
    int sockfd;
    char buffer[MAXBUFSIZE];
    char sendBuffer[MAXBUFSIZE];
    socklen_t addrLen = sizeof(struct sockaddr);

public:
    UdpSocket();
    UdpSocket(int sockfd);
    int initSocket();
    struct Package recvMsg();
    struct Package recvMsgWithTimeOut();
    struct Package recvMsg(struct sockaddr* addr);
    int sendMsg(struct Message msg, struct sockaddr* addr);
    int sendMsg(struct Package pkg, struct sockaddr* addr);
    int sendMsg(int seq, int randomId, std::string s, struct sockaddr* addr);
    int shutDownSocket();
    char genCheckSum(struct Message msg);
    char genCheckSum(char* buf, int len);
    bool examineCheckSum(struct Message msg);
    std::string vectorToString(std::vector<char> input);
};