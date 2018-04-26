#include "client.h"

void Client::startClient(){
    this->udpSock = NULL;
    this->udpSock = initSocket();
    if (this->udpSock == NULL){
        return;
    }

    this->flag = false;

    std::thread sendThread = std::thread([&](){
        int i;
        while (true){
            if (scanf("%d",&i) != -1){
                if (i == -1){
                    this->flag = true;
                    break;
                }
                if (i <= 0) continue;
                if (this->sendNumber(i) == 0){
                    printf("Successfully sent number: %d\n",i);
                }
            }
        }
    });

    std::thread recvThread = std::thread([&](){
        std::string s;
        while (true){
            if (this->flag) break;
            this->recvRes = this->recvMsg();
            if (this->recvRes.size() == 0){
                continue;
            } else {
                s = this->udpSock->vectorToString(this->recvRes);
                printf("Received data from server:\n%s\n",s.c_str());
            }
        }
    });

    sendThread.join();
    recvThread.join();

    this->udpSock->shutDownSocket();
    delete(this->udpSock);
}

UdpSocket* Client::initSocket(){
    int sock;  
 
    if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0){  
        perror("socket error");  
        return NULL;
    }  
 
    memset(&servAddr, 0, sizeof(servAddr));  
 
    servAddr.sin_family = AF_INET;  
    servAddr.sin_port = htons(SERVERPORT);  
    servAddr.sin_addr.s_addr = inet_addr(SERVERIPADDR);  
    
    UdpSocket* s = new UdpSocket(sock);
    return s;
}

int Client::sendNumber(int num){
    if (num <= 0) return 1;

    struct Message msg;
    union NumberMsg nm;
    std::random_device rd;
    std::uniform_int_distribution<int> dis(0,10000);

    nm.num = htonl(num);
    msg.sequenceNum = 1;
    msg.length = 4+HEADERLEN;
    msg.randomId = dis(rd);
    msg.data.resize(4);
    memcpy(msg.data.data(),nm.dat,4);
    
    int ret = this->udpSock->sendMsg(msg,(struct sockaddr*)&servAddr);
    if (ret != 0){
        return 1;
    }
    return 0;
}

std::vector<char> Client::recvMsg(){
    struct Package pkg = this->udpSock->recvMsg();
    std::vector<char> ans;
    ans.resize(0);

    //检测来源是否正确
    if ((pkg.peerAddr.sin_port != this->servAddr.sin_port) || 
        (pkg.peerAddr.sin_addr.s_addr != this->servAddr.sin_addr.s_addr)){
        perror("invalid server!");
        return ans;
    }

    //接收错误或断开连接？
    if (pkg.msg.length == -1){
        return ans;
    }

    //检测数据长度是否正确
    if (pkg.msg.length != pkg.msg.data.size()+HEADERLEN){
        perror("invalid data!");
        return ans;
    }

    ans = pkg.msg.data;
    return ans;
}