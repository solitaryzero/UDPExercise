#include "server.h"

void Server::startServer(){
    this->udpSock = NULL;
    this->udpSock = initSocket();
    if (this->udpSock == NULL){
        return;
    }

    while (true){
        Package p = recvPkg();
        sendString(p);
    }

    this->udpSock->shutDownSocket();
    delete(this->udpSock);
}

UdpSocket* Server::initSocket(){
    int sock;  
 
    if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){  
        perror("socket error");  
        return NULL;
    }  
 
    struct sockaddr_in servAddr;  
    memset(&servAddr, 0, sizeof(servAddr));  
 
    servAddr.sin_family = AF_INET;  
    servAddr.sin_port = htons(SERVERPORT);  
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    
    if(bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0){  
        perror("bind error");  
        return NULL;
    }
    
    UdpSocket* s = new UdpSocket(sock);
    return s;
}

struct Package Server::recvPkg(){
    return this->udpSock->recvMsg();
}

int Server::sendString(struct Package p){

    //判断是否是一个合法的数据包(内容为一个int)
    if ((p.msg.length <= HEADERLEN) || (p.msg.length-HEADERLEN != 4) || (p.msg.data.size() != 4)){
        printf("%d\n",p.msg.length-HEADERLEN);
        printf("%d\n",p.msg.data.size());
        perror("bad package");
        return 1;
    }
    
    union NumberMsg nm;
    memcpy(nm.dat,p.msg.data.data(),4);
    int l = ntohl(nm.num);
    printf("Inquiring number: %d\n",l);

    //判断请求的字符串长度是否合法
    if ((l > MAXSTRINGLEN) || (l <= 0)){
        printf("bad string length: %d\n",l);
        return 2;
    }

    //通过std::random创建随机字符串
    std::vector<char> builder;
    builder.clear();
    std::random_device rd;
    std::uniform_int_distribution<int> dis(0,25);
    for (int i=0;i<l;i++){
        builder.push_back((char)(dis(rd)+(int)('A')));
    }

    //根据原请求设置对应字段
    Message msg;
    msg.sequenceNum = p.msg.sequenceNum;
    msg.length = l+HEADERLEN;
    msg.randomId = p.msg.randomId;
    msg.data = builder;
    if (this->udpSock->sendMsg(msg,(struct sockaddr*)&(p.peerAddr)) == 0){
        return 0;
    } else {
        return 1;
    }

}