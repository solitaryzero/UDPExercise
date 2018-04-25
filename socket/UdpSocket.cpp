#include "UdpSocket.h"

UdpSocket::UdpSocket(){
    this->sockfd = -1;
}

UdpSocket::UdpSocket(int sockfd){
    this->sockfd = sockfd;
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

    if (pkgSize <= MSGOFFSET){  
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    pkg.msg.sequenceNum = ntohl(((int*)buffer)[0]);
    pkg.msg.length = ntohl(((int*)buffer)[1]);

    if (pkg.msg.length != pkgSize-MSGOFFSET){
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    }

    std::vector<char> res;
    res.resize(pkg.msg.length);
    memcpy(res.data(), buffer+MSGOFFSET, pkg.msg.length);
    pkg.msg.data = res;
    pkg.peerAddr = addr;

    return pkg;
}

struct Package UdpSocket::recvMsg(struct sockaddr* addr){
    struct Package pkg;
    int pkgSize = recvfrom(this->sockfd, buffer, sizeof(buffer), 0, addr, &addrLen);  

    if (pkgSize == -1){  
        perror("recvfrom error!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    if (pkgSize <= MSGOFFSET){  
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    } 

    pkg.msg.sequenceNum = ntohl(((int*)buffer)[0]);
    pkg.msg.length = ntohl(((int*)buffer)[1]);

    if (pkg.msg.length != pkgSize-MSGOFFSET){
        perror("invalid package!");  
        pkg.msg.length = -1;
        return pkg;
    }

    std::vector<char> res;
    res.resize(pkg.msg.length);
    memcpy(res.data(), buffer+MSGOFFSET, pkg.msg.length);
    pkg.msg.data = res;
    pkg.peerAddr = *(struct sockaddr_in*)(addr);

    return pkg;
}

int UdpSocket::sendMsg(struct Message msg, struct sockaddr* addr){
    if (msg.length >= MAXBUFSIZE-MSGOFFSET){
        return -2;
    }

    ((int*)sendBuffer)[0] = htonl(msg.sequenceNum);
    ((int*)sendBuffer)[1] = htonl(msg.length);
    memcpy(sendBuffer+MSGOFFSET, msg.data.data(), msg.length);
    int ret = sendto(sockfd, sendBuffer, msg.length+MSGOFFSET, 0, addr, addrLen);
    if (ret == -1){
        perror("sendMsg error!");
        return -1;
    }
    return 0;
}

int UdpSocket::sendMsg(int seq, std::string s, struct sockaddr* addr){
    ((int*)sendBuffer)[0] = htonl(seq);
    ((int*)sendBuffer)[1] = htonl(s.length()+1);
    memcpy(sendBuffer+MSGOFFSET, s.c_str(), s.length()+1);
    int ret = sendto(sockfd, sendBuffer, s.length()+1+MSGOFFSET, 0, addr, addrLen);
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