#include "client.h"

void Client::startClient(){
    this->udpSock = NULL;
    this->udpSock = initSocket();
    if (this->udpSock == NULL){
        return;
    }

    this->flag = false;
    this->currentSequenceNum = 0;
    this->retryCount = 0;
    this->packMap.clear();
    while (!this->packList.empty()){
        this->packList.pop_back();
    }

/*
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
                    printf("Successfully sent number with seq num %d: %d\n",this->currentSequenceNum-1, i);
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
*/

    int i,se;
    std::string s;
    while (true){
        if (scanf("%d",&i) != -1){
            if (i == -1){
                this->flag = true;
                break;
            }
            if ((se = this->sendNumber(i)) >= 0){
                this->retryCount = 0;
                printf("Successfully sent number with seq num %d: %d\n",se , i);
                this->recvRes = this->recvMsg(se);
                if (this->recvRes.size() == 0){
                    continue;
                } else {
                    s = this->udpSock->vectorToString(this->recvRes);
                    printf("Received data from server:\n%s\n",s.c_str());
                }
            }
        }
    }

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
    if ((num <= 0) || (num > MAXSTRINGLEN)) {
        perror("invalid number!");
        return -1;
    }

    struct Message msg;
    union NumberMsg nm;

    //下一个序列号是否可用?
    int next = this->currentSequenceNum;
    while (next != this->currentSequenceNum+MAXSEQNUM){
        if (this->packMap.count(next%MAXSEQNUM) == 0){
            break;
        }
        next = next+1;
    }

    if (next == this->currentSequenceNum+MAXSEQNUM){
        perror("sequence number used up!");
        return -1;
    }

    msg.sequenceNum = next%MAXSEQNUM;
    this->currentSequenceNum = next%MAXSEQNUM+1;

    std::random_device rd;
    std::uniform_int_distribution<int> dis(0,10000);

    nm.num = htonl(num);

    //填写字段
    msg.length = 4+HEADERLEN;
    msg.randomId = dis(rd);
    msg.data.resize(4);
    memcpy(msg.data.data(),nm.dat,4);

    int ret = this->udpSock->sendMsg(msg,(struct sockaddr*)&servAddr);
    if (ret != 0){
        return -1;
    }

    //设置序列号对应关系
    this->packMap[msg.sequenceNum] = msg;
    PackData pd;
    pd.seq = msg.sequenceNum;
    pd.retryCount = 0;
    pd.waitTime = MAXWAITTIME;

    this->mtx.lock();
    this->packList.push_back(pd);
    this->mtx.unlock();

    return msg.sequenceNum;
}

std::vector<char> Client::recvMsg(int seq){
    struct Package pkg = this->udpSock->recvMsgWithTimeOut();
    std::vector<char> ans;
    ans.resize(0);

    //超时
    if (pkg.msg.length == -2){
        this->retryCount++;
        if (this->retryCount > MAXRETRY){
            printf("Too many retries, drop package.\n");
            this->packMap.erase(pkg.msg.sequenceNum);
            return ans;
        }
        if (this->udpSock->sendMsg(this->packMap[seq],(struct sockaddr*)&servAddr) == 0){
            printf("Package with seq num %d timed out. Resending for %d times...\n", seq, this->retryCount);
            return this->recvMsg(seq);
        }
    }

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

    //检测是否发送了该请求
    if (this->packMap.count(pkg.msg.sequenceNum) == 0){
        perror("unexpected message!");
        return ans;
    }

    //检测随机数是否一致
    if (this->packMap[pkg.msg.sequenceNum].randomId != pkg.msg.randomId){
        perror("bad random id!");
        return ans;
    }

    int strLen = *(int*)(this->packMap[pkg.msg.sequenceNum].data.data());
    printf("Received response with sequence number %d, aka string with length of %d\n", 
    pkg.msg.sequenceNum,ntohl(strLen));
    this->packMap.erase(pkg.msg.sequenceNum);

    ans = pkg.msg.data;
    return ans;
}