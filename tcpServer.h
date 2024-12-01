#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>

class TCPserver {
public:
    TCPserver();
    ~TCPserver();
    void startListen();

private:
    int startServer();
    void sendResponse();

    addrinfo hints, *res;
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
    char* response;
    int res_len;
};
