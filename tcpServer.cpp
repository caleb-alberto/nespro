#include "tcpServer.h"

TCPserver::TCPserver() {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("0.0.0.0", "8080", &hints, &res); // NOLINT to ignore warning

    int server = startServer();

    if (server == 1) {
        std::cerr << "Socket was unable to be created\n";
        exit(1);
    }
    else if (server == -1) {
        std::cerr << "Socket was unable to bind\n";
        exit(1);
    }
}

TCPserver::~TCPserver() {
    freeaddrinfo(res);
}

int TCPserver::startServer() {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) { return 1; }
    return bind(sockfd, res->ai_addr, res->ai_addrlen);
}

void TCPserver::startListen() {
    if (listen(sockfd, 20) == -1) {
        std::cerr << "Unable to listen\n";
        exit(1);
    }

    while(true) {
        addr_size = sizeof(client_addr);
        client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);
        if (client_sockfd == -1) {
            std::cerr << "Unable to accept\n";
            exit(1);
        }
    }
}
