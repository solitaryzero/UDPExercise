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

   std::thread sendThread = std::thread([&](){
        int i;
        while (true){
            if (scanf("%d",&i) != -1){
                if (i == -1){
                    this->flag = true;
                    break;
                }
                if (i <= 0) continue;
                if (this->sendNumber(i) >= 0){
                    printf("* Successfully sent number with seq num %d: %d\n",this->currentSequenceNum-1, i);
                }
            }
        }
    });

    std::thread recvThread = std::thread([&](){
        std::string s;
        while (true){
            if (this->flag) break;
            this->recvRes = this->recvMsg(0);
            if (this->recvRes.size() == 0){
                continue;
            } else {
                s = this->udpSock->vectorToString(this->recvRes);
                printf("* Received data from server:\n%s\n",s.c_str());
            }
        }
    });

    std::thread listManageThread = std::thread([&](){
        while (true){
            while ((!this->packList.empty()) && (this->packMap.count(this->packList.front().seq) == 0)){
                this->listMtx.lock();
                this->packList.pop_front();
                this->listMtx.unlock();
            }

            time_t timeNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            int maxCnt = this->packList.size();
            while ((maxCnt > 0) && (!this->packList.empty())){
                if (this->packMap.count(this->packList.front().seq) != 0) {
                    struct PackData pd = this->packList.front();
                    this->packList.pop_front();
                    if (timeNow - pd.startTime > MAXWAITTIME){
                        pd.retryCount++;
                        pd.startTime = timeNow;
                        if (pd.retryCount > MAXRETRY){
                            this->mapMtx.lock();
                            this->packMap.erase(pd.seq);
                            printf("# Too many retries, drop package No.%d .\n", pd.seq);
                            this->mapMtx.unlock();
                        } else {
                            this->listMtx.lock();
                            this->packList.push_back(pd);
                            this->listMtx.unlock();
                            printf("# Package with seq num %d timed out. Resending for %d times...\n", pd.seq, pd.retryCount);
                            this->resendNumber(pd.seq);
                        }
                    } else {
                        this->listMtx.lock();
                        this->packList.push_back(pd);
                        this->listMtx.unlock();
                    }
                }
                maxCnt--;
            }

            sleep(1);
        }
    });

    sendThread.join();
    recvThread.join();
    listManageThread.join();

/*  
    // below are code for blocking send and receive.
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
                printf("* Successfully sent number with seq num %d: %d\n",se , i);
                this->recvRes = this->recvMsg(se);
                if (this->recvRes.size() == 0){
                    continue;
                } else {
                    s = this->udpSock->vectorToString(this->recvRes);
                    printf("* Received data from server:\n%s\n",s.c_str());
                }
            }
        }
    }
*/

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

    this->sockMtx.lock();
    int ret = this->udpSock->sendMsg(msg,(struct sockaddr*)&servAddr);
    this->sockMtx.unlock();
    if (ret != 0){
        return -1;
    }

    //设置序列号对应关系
    this->mapMtx.lock();
    this->packMap[msg.sequenceNum] = msg;
    this->mapMtx.unlock();

    PackData pd;
    pd.seq = msg.sequenceNum;
    pd.retryCount = 0;
    pd.startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    this->listMtx.lock();
    this->packList.push_back(pd);
    this->listMtx.unlock();

    return msg.sequenceNum;
}

int Client::resendNumber(int seq){
    struct Message msg = this->packMap[seq];
    this->sockMtx.lock();
    this->udpSock->sendMsg(msg,(struct sockaddr*)(&this->servAddr));    
    this->sockMtx.unlock();
}

std::vector<char> Client::recvMsg(int seq){
    std::vector<char> ans;
    ans.resize(0);

    /*
    // below are code for blocking send and receive.
    struct Package pkg = this->udpSock->recvMsgWithTimeOut();

    //超时
    if (pkg.msg.length == -2){
        this->retryCount++;
        if (this->retryCount > MAXRETRY){
            printf("# Too many retries, drop packagev No.%d .\n", seq);
                this->mapMtx.lock();
                this->packMap.erase(pkg.msg.sequenceNum);
                this->mapMtx.unlock();
            return ans;
        }
        if (this->udpSock->sendMsg(this->packMap[seq],(struct sockaddr*)&servAddr) == 0){
            printf("# Package with seq num %d timed out. Resending for %d times...\n", seq, this->retryCount);
            return this->recvMsg(seq);
        }
    }
    */

    struct Package pkg = this->udpSock->recvMsg();

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
    printf("* Received response with sequence number %d, aka string with length of %d\n", 
    pkg.msg.sequenceNum,ntohl(strLen));
    
    this->mapMtx.lock();
    this->packMap.erase(pkg.msg.sequenceNum);
    this->mapMtx.unlock();

    ans = pkg.msg.data;
    return ans;
}