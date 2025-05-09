// fix 80px indenting on buffer
// set up inheritence for HTTPS_server class

#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <curl/curl.h>
#include <curl/easy.h>
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
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>

struct Request {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> header_map;
    std::string body;
};

class HTTPserver {
public:
    HTTPserver(std::string port = "8080", std::string html_dir = "");
    ~HTTPserver();
    void startListen(std::string backend_url);

protected:
    int startServer();
    void sendResponse(std::string response);
    std::string forwardResponse(Request dynamic_req, std::string backend_url);
    std::string buildRes(const Request & msg, std::string req_path);
    std::string recvReq(const int socket);
    Request parseReq(std::string req);
    std::unordered_map<std::string, std::string> parseStatDir(std::string dir);

    addrinfo hints, *res;
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
    std::string response;
    std::string req_str;
    int res_len;
    std::unordered_map<std::string, std::string> endpoints;
};

#endif
