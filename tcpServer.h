#include <map>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>

struct Request {
    std::string method;
    std::string path;
};

class TCPserver {
public:
    TCPserver(std::string port = "8080");
    ~TCPserver();
    void startListen();

private:
    int startServer();
    void sendResponse();
    std::string buildRes(const Request & msg);

    addrinfo hints, *res;
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
    std::string response;
    int res_len;
};
