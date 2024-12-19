#include "tcpServer.h"
using namespace std;

int main() {
    TCPserver server;
    ifstream index("./index.html");

    server.startListen(index);
}
