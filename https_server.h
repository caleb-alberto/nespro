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
    int acceptConnection(const int socket) override;
    void closeConnection(const int client) override;
    std::string recvClient(char* buf, size_t size) override;
    ssize_t writeClient(const char* buf, const size_t size) override;
};

#endif
