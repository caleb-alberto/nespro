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
#include <time.h>
#include <thread>

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
    double time;
    size_t read;
    size_t sent;
};

class HTTPserver {
public:
    HTTPserver(std::string max_connections,
               std::string port = "8080",
               const std::string dir = "");
    ~HTTPserver();
    void startListen(std::string backend_url);

protected:
    int startServer();
    virtual void acceptConnection(int& client_sockfd, Message& msg);
    virtual void closeConnection(const int client_sockfd);
    void performThread(int client_sockfd,
                       Message& msg,
                       std::string backend_url);
    virtual ssize_t recvClient(int client_sockfd, char* buf, size_t size);
    ssize_t recvReq(int client_sockfd, Message& msg, std::string& req_str);
    virtual ssize_t writeClient(int client_sockfd,
                                const char* buf,
                                const size_t size);
    void sendResponse(int client_sockfd, std::string response, Message& msg);
    void logMessage(Message& msg);
    Request parseReq(std::string req);
    std::unordered_map<std::string, std::string> parseStatDir(std::string dir);
    void buildRes(int client_sockfd,
                  std::string method,
                  std::string req_path,
                  Message& msg);
    std::string forwardResponse(Request dynamic_req, std::string backend_url);

    int connections;
    int max_connections;
    int lfd;
    addrinfo hints, *res;
    socklen_t addr_size;
    int sockfd;
    size_t res_len;
    std::unordered_map<std::string, std::string> endpoints;
    bool conditional;
};

#endif
