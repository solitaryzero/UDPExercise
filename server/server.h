#include "../socket/UdpSocket.h"
#include <random>

class Server{
private:
    UdpSocket* udpSock;
    struct sockaddr_in servaddr;  

public:
    void startServer();
    UdpSocket* initSocket();
    struct Package recvPkg();
    int sendString(struct Package p);
};