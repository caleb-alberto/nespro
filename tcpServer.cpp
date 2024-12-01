#include "tcpServer.h"

TCPserver::TCPserver(char* port) {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("0.0.0.0", port, &hints, &res); // NOLINT

    int server = startServer();

    if (server == 1) {
        std::cerr << "Socket was unable to be created\n";
        exit(1);
    }
    else if (server == -1) {
        std::cerr << "Socket was unable to bind\n";
        exit(1);
    }
    else {
        std::cout << "Server listening on Port: " << port << std::endl;
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
    std::cout << "Socket is listening\n";

    while(true) {
        addr_size = sizeof(client_addr);
        client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

        if (client_sockfd == -1) {
            std::cerr << "Unable to accept\n";
            exit(1);
        }

        sockaddr_in* client_in = (sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected from IP: " << client_ip << std::endl;

        response = "HTTP/1.1 200 OK\r\n"; //NOLINT
        res_len = strlen(response);

        for (int total_sent = 0; total_sent < res_len; ) {
            int bytes_sent = send(client_sockfd, response + total_sent, res_len - total_sent, 0);

            if (bytes_sent == -1) {
                std::cerr << "Unable to send\n";
                exit(1);
            }
            total_sent += bytes_sent;
        }

        close(client_sockfd);
    }
}
