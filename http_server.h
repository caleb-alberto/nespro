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
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <format>

struct Request {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> header_map;
    std::string body;
};

struct Message {
    bool error;
    std::string ip;
    std::string method;
    std::string path;
    std::string version;
    std::string user_agent;
    std::string host;
    std::string tls_version;
    std::string error_msg;
    size_t read;
    size_t sent;
};

class HTTPserver {
public:
    HTTPserver(std::string port = "8080", const std::string dir = "");
    ~HTTPserver();
    void startListen(std::string backend_url);

protected:
    int startServer();
    virtual void acceptConnection(const int socket);
    virtual void closeConnection(const int client);
    virtual ssize_t recvClient(char* buf, size_t size);
    ssize_t recvReq();
    virtual ssize_t writeClient(const char* buf, const size_t size);
    void sendResponse(std::string response);
    void logMessage();
    Request parseReq(std::string req);
    std::unordered_map<std::string, std::string> parseStatDir(std::string dir);
    void buildRes(std::string method, std::string req_path);
    std::string forwardResponse(Request dynamic_req, std::string backend_url);

    Message msg;
    int lfd;
    addrinfo hints, *res;
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int client_sockfd;
    std::string response;
    std::string req_str;
    size_t res_len;
    std::unordered_map<std::string, std::string> endpoints;
    bool conditional;
};

#endif
