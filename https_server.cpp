#include "https_server.h"

HTTPSserver::HTTPSserver(std::string port = "8080",
                         std::string html_dir = "",
                         std::string cert_file = "")
    : HTTPserver(port, html_dir) {
    
}
