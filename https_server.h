#ifndef HTTPS_SERVER
#define HTTPS_SERVER

#include "http_server.h"
#include <openssl/ssl.h>

class HTTPSserver : public HTTPserver {
public:
    HTTPSserver(std::string port = "8080",
                std::string dir = "",
                const char* cert_file = "",
                const char* prv_file = "");
    ~HTTPSserver();

private:
    void acceptConnection(int& client_sockfd, Message& msg) override;
    void closeConnection(const int client) override;
    ssize_t recvClient(int client_sockfd, char* buf, size_t size) override;
    ssize_t writeClient(int client_sockfd,
                        const char* buf,
                        const size_t size) override;

    SSL_CTX *sslctx;
    SSL *cSSL;
};

#endif
