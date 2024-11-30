#include "tcpServer.h"
#include <sys/socket.h>

int main() {
    TCPserver server;
    server.startListen();
}
