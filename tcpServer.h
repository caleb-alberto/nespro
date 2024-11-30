#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>

class TCPserver {
public:
    TCPserver();
    ~TCPserver();
    void startListen();

private:
    int startServer();

    addrinfo hints, *res;
    int sockfd;
};
