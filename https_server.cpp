#include "https_server.h"
using namespace std;

HTTPSserver::HTTPSserver(string port,
                         string html_dir,
                         const char* cert_file,
                         const char* prv_file)
    : HTTPserver(port, html_dir) {
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    sslctx = SSL_CTX_new( SSLv23_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_ALL);

    if (SSL_CTX_use_certificate_file(sslctx, cert_file, SSL_FILETYPE_PEM) != 1) {
        cerr << "Certificate file not valid\n";
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(sslctx, prv_file, SSL_FILETYPE_PEM) != 1) {
        cerr << "Private key file not valid\n";
        exit(1);
    }

    cSSL = SSL_new(sslctx);
}

HTTPSserver::~HTTPSserver() {
    SSL_shutdown(cSSL);
    SSL_free(cSSL);
    SSL_CTX_free(sslctx);
}
