#include "http_server.h"
using namespace std;

int main(int argc, char** argv) {
    HTTPserver server("8080", "html_files/");
    server.startListen("http://localhost:3000");
}
