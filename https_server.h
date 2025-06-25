#ifndef HTTPS_SERVER
#define HTTPS_SERVER

#include "http_server.h"

class HTTPSserver : public HTTPserver {
public:
    HTTPSserver(std::string port = "8080",
                std::string html_dir = "",
                std::string cert_file = "");
    ~HTTPSserver();

private:
    int acceptConnection(const int socket);
    void closeConnection(const int client);
    std::string recvReq(const int socket);
    void sendResponse(std::string response);
};

#endif
