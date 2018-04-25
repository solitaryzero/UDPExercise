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
    struct Package recvMsg();
    struct Package recvMsg(struct sockaddr* addr);
    int sendMsg(struct Message msg, struct sockaddr* addr);
    int sendMsg(struct Package pkg, struct sockaddr* addr);
    int sendMsg(int seq, std::string s, struct sockaddr* addr);
    int shutDownSocket();
    std::string vectorToString(std::vector<char> input);
};