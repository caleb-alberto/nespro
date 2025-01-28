#include "tcpServer.h"
using namespace std;

int main() {
    TCPserver server("8080", "html_files/");
    server.startListen("http://localhost:3000");
}
