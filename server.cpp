#include "https_server.h"
using namespace std;

int main(int argc, char** argv) {
    HTTPSserver server("8080",
                       "html_files/",
                       "/etc/letsencrypt/live/calebalberto.duckdns.org/fullchain.pem",
                       "/etc/letsencrypt/live/calebalberto.duckdns.org/privkey.pem");
    server.startListen("http://localhost:3000");
}
