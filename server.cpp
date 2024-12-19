#include "tcpServer.h"
using namespace std;

int main() {
    TCPserver server;
    server.startListen("./index.html");
}
