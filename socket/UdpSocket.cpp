#include "UdpSocket.h"

UdpSocket::UdpSocket(){
    this->sockfd = -1;
}

UdpSocket::UdpSocket(int sockfd){
    this->sockfd = sockfd;
    initSocket();
}

int UdpSocket::initSocket(){
    int reuseFlag = 1;
    setsockopt(this->sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseFlag,sizeof(reuseFlag)); 
    setsockopt(this->sockfd,SOL_SOCKET,SO_REUSEPORT,&reuseFlag,sizeof(reuseFlag)); 

    return 0;
}

struct Package UdpSocket::recvMsg(){
    struct Package pkg;
    struct sockaddr_in addr; 

    int pkgSize = recvfrom(this->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrLen);  

    if (pkgSize == -1){  
        perror("recvfrom error!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    if (pkgSize <= HEADERLEN){  
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    pkg.msg.sequenceNum = ntohl(((int*)buffer)[0]);
    pkg.msg.length = ntohl(((int*)buffer)[1]);
    pkg.msg.randomId = ntohl(((int*)buffer)[2]);
    pkg.msg.checkSum = buffer[HEADERLEN-1];

    if (pkg.msg.length != pkgSize){
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    }

    std::vector<char> res;
    res.resize(pkg.msg.length-HEADERLEN);
    memcpy(res.data(), buffer+HEADERLEN, pkg.msg.length-HEADERLEN);
    pkg.msg.data = res;

    if (!examineCheckSum(pkg.msg)){
        perror("bad checksum!");
        pkg.msg.length = -1;
        return pkg;
    }

    pkg.peerAddr = addr;

    return pkg;
}

struct Package UdpSocket::recvMsgWithTimeOut(){
    struct timeval tv;
    fd_set readfds;
    FD_ZERO(&readfds); 
    FD_SET(this->sockfd,&readfds);
    tv.tv_sec=MAXWAITTIME; tv.tv_usec=0;
    if (select(this->sockfd+1,&readfds,NULL,NULL,&tv) > 0) {
        if (FD_ISSET(this->sockfd,&readfds)){
            return recvMsg();
        }
    } else {
        Package pkg;
        pkg.msg.length = -2;
        return pkg;
    }
}

struct Package UdpSocket::recvMsg(struct sockaddr* addr){
    struct Package pkg;
    int pkgSize = recvfrom(this->sockfd, buffer, sizeof(buffer), 0, addr, &addrLen);  

    if (pkgSize == -1){  
        perror("recvfrom error!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    if (pkgSize <= HEADERLEN){  
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    pkg.msg.sequenceNum = ntohl(((int*)buffer)[0]);
    pkg.msg.length = ntohl(((int*)buffer)[1]);
    pkg.msg.randomId = ntohl(((int*)buffer)[2]);
    pkg.msg.checkSum = buffer[HEADERLEN-1];

    if (pkg.msg.length != pkgSize){
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    }

    std::vector<char> res;
    res.resize(pkg.msg.length-HEADERLEN);
    memcpy(res.data(), buffer+HEADERLEN, pkg.msg.length-HEADERLEN);
    pkg.msg.data = res;

    if (!examineCheckSum(pkg.msg)){
        perror("bad checksum!");
        pkg.msg.length = -1;
        return pkg;
    }

    pkg.peerAddr = *(struct sockaddr_in*)(addr);

    return pkg;
}

int UdpSocket::sendMsg(struct Message msg, struct sockaddr* addr){
    if (msg.length >= MAXBUFSIZE){
        return -2;
    }

    ((int*)sendBuffer)[0] = htonl(msg.sequenceNum);
    ((int*)sendBuffer)[1] = htonl(msg.length);
    ((int*)sendBuffer)[2] = htonl(msg.randomId);
    memcpy(sendBuffer+HEADERLEN, msg.data.data(), msg.length-HEADERLEN);
    ((char*)sendBuffer)[HEADERLEN-1] = genCheckSum(msg);

    int ret = sendto(sockfd, sendBuffer, msg.length, 0, addr, addrLen);
    if (ret == -1){
        perror("sendMsg error!");
        return -1;
    }
    return 0;
}

int UdpSocket::sendMsg(int seq, int randomId, std::string s, struct sockaddr* addr){
    ((int*)sendBuffer)[0] = htonl(seq);
    ((int*)sendBuffer)[1] = htonl(s.length()+1+HEADERLEN);
    ((int*)sendBuffer)[2] = htonl(randomId);
    memcpy(sendBuffer+HEADERLEN, s.c_str(), s.length()+1);
    ((char*)sendBuffer)[HEADERLEN-1] = genCheckSum(sendBuffer,s.length()+1+HEADERLEN);

    int ret = sendto(sockfd, sendBuffer, s.length()+1+HEADERLEN, 0, addr, addrLen);
    if (ret == -1){
        perror("sendMsg error!");
        return -1;
    }
    return 0;
}

int UdpSocket::sendMsg(struct Package pkg, struct sockaddr* addr){
    return sendMsg(pkg.msg,addr);
}

int UdpSocket::shutDownSocket(){
    shutdown(this->sockfd,2);
    return 0;
}

std::string UdpSocket::vectorToString(std::vector<char> input){
    std::string s(input.begin(),input.end());
    return s;
}

char UdpSocket::genCheckSum(struct Message msg){
    msg.checkSum = 0;
    char res = 0;
    for (int i=0;i<HEADERLEN;i++){
        res = res ^ ((char*)&msg)[i];
    }

    int len = msg.length-HEADERLEN;
    for (int i=0;i<len;i++){
        res = res ^ msg.data[i];
    }
    return res;
}

char UdpSocket::genCheckSum(char* buf, int len){
    ((char*)buf)[HEADERLEN-1] = 0;
    char res = 0;
    for (int i=0;i<len;i++){
        res = res ^ buf[i];
    }

    return res;
}

bool UdpSocket::examineCheckSum(struct Message msg){
    char res = 0;
    for (int i=0;i<HEADERLEN;i++){
        res = res ^ ((char*)&msg)[i];
    }

    int len = msg.length-HEADERLEN;
    for (int i=0;i<len;i++){
        res = res ^ msg.data[i];
    }
    if (res == 0){
        return true;
    } else {
        return false;
    }
}