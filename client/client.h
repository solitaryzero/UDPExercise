#include "../socket/UdpSocket.h"
#include <thread>

class Client{
private:
    UdpSocket* udpSock;
    struct sockaddr_in servAddr;  
    std::vector<char> recvRes;
    bool flag;

public:
    void startClient();
    UdpSocket* initSocket();
    int sendNumber(int num);
    std::vector<char> recvMsg();
};