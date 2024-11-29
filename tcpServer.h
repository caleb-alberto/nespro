#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class TCPserver {
public:
    TCPserver(std::string ip, int port);
    ~TCPserver();
    void startListen();

private:
    int startServer();

    sockaddr_in mSocketAddr;
    int mSocket;
};
