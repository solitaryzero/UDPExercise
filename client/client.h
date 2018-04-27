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

class Client{
private:
    UdpSocket* udpSock;
    struct sockaddr_in servAddr;  
    std::map<int,struct Message> packMap;
    std::deque<struct PackData> packList;
    std::vector<char> recvRes;
    std::mutex mtx;
    int nextSequenceNum;
    int retryCount;
    bool flag;

public:
    void startClient();
    UdpSocket* initSocket();
    int sendNumber(int num);
    std::vector<char> recvMsg(int seq);
};