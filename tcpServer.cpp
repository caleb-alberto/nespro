#include "tcpServer.h"
#include <sys/socket.h>

TCPserver::TCPserver(std::string ip, int port) {
    mSocketAddr.sin_family = AF_INET;
    mSocketAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(mSocketAddr.sin_addr));

    if (startServer() != 0) {
        //error handle
    }

}

TCPserver::~TCPserver() {
}

int TCPserver::startServer() {
    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    return bind(mSocket, (const struct sockaddr *)&mSocketAddr, sizeof(mSocketAddr));
}

void TCPserver::startListen() {
}
