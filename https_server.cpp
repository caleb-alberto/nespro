#include "https_server.h"
using namespace std;

HTTPSserver::HTTPSserver(string max_connections,
                         string port,
                         string dir,
                         const char* cert_file,
                         const char* prv_file)
    : HTTPserver(max_connections, port, dir) {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    sslctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_ALL);

    if (SSL_CTX_use_certificate_file(sslctx, cert_file, SSL_FILETYPE_PEM) != 1) {
        cerr << "Certificate file not valid\n";
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(sslctx, prv_file, SSL_FILETYPE_PEM) != 1) {
        cerr << "Private key file not valid\n";
        exit(1);
    }
}

HTTPSserver::~HTTPSserver() {
    SSL_CTX_free(sslctx);
}

void HTTPSserver::acceptConnection(int& client_sockfd, Message& msg) {
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    addr_size = sizeof(client_addr);

    client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

    if (client_sockfd < 1) {
        msg.error = true;
        msg.error_msg = "Unable to accept";
        conditional = false;

        return;
    }
    sockaddr_in* client_in = (sockaddr_in*)&client_addr;
    inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
    msg.ip = string(client_ip);

    SSL *cSSL = SSL_new(sslctx);
    SSL_set_fd(cSSL, client_sockfd);

    if (ssl_connections.size() <= client_sockfd)
        ssl_connections.resize(client_sockfd + 1);
    ssl_connections[client_sockfd] = cSSL;

    if (SSL_accept(cSSL) < 1) {
        msg.error = true;
        msg.error_msg = "Unsuccessful TLS/SSL handshake";
        conditional = false;
    }
    
    msg.tls_version = string(SSL_get_version(cSSL));
}

void HTTPSserver::closeConnection(const int client_sockfd) {
    SSL *cSSL = ssl_connections[client_sockfd];
    ssl_connections[client_sockfd] = nullptr;

    SSL_shutdown(cSSL);
    SSL_free(cSSL);

    close(client_sockfd);
}

ssize_t HTTPSserver::recvClient(int client_sockfd, char* buf, size_t size) {
    return SSL_read(ssl_connections[client_sockfd], buf, size);
}

ssize_t HTTPSserver::writeClient(int client_sockfd,
                                 const char* buf,
                                 const size_t size) {
    return SSL_write(ssl_connections[client_sockfd], buf, size);
}
