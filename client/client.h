#include "../socket/UdpSocket.h"
#include <thread>
#include <random>
#include <map>

class Client{
private:
    UdpSocket* udpSock;
    struct sockaddr_in servAddr;  
    std::map<int,struct Package> packMap;
    std::vector<char> recvRes;
    int nextSequenceNum;
    bool flag;

public:
    void startClient();
    UdpSocket* initSocket();
    int sendNumber(int num);
    std::vector<char> recvMsg();
};