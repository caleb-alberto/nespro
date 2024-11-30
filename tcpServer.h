#include <string>
#include <sys/socket.h>
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
    sockaddr_storage client_addr;
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
};
