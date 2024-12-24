#include <fstream>
#include <filesystem>
#include <map>
#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>

struct Request {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> header_map;
};

class TCPserver {
public:
    TCPserver(std::string port = "8080", std::string html_dir = "");
    ~TCPserver();
    void startListen();

private:
    int startServer();
    void sendResponse();
    std::string buildRes(const Request & msg, std::vector<std::string> paths);
    std::string recvReq(const int socket);
    Request parseReq(std::string req);
    std::vector<std::string> parseStatDir(std::string dir);

    std::string dir;
    addrinfo hints, *res;
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
    std::string response;
    std::string req_str;
    int res_len;
};
