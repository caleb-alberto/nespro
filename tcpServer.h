#include <sys/socket.h>

namespace https {
    class TCPserver {
    public:
        TCPserver();
        ~TCPserver();
        void startListen();
    };
}
