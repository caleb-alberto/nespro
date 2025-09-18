#include "https_server.h"
#include <yaml-cpp/yaml.h>

using namespace std;

int main(int argc, char** argv) {
    YAML::Node config = YAML::LoadFile("config.yaml");

    const string max_connections = config["max connections"].as<string>();
    const string port = config["port"].as<string>();
    const string public_dir = config["public folder"].as<string>();
    const string cert_file = config["certificate file"].as<string>();
    const string key_file =  config["private key file"].as<string>();
    const string backend_url = config["backend url"].as<string>();

    HTTPSserver server(max_connections,
                       port,
                       public_dir,
                       cert_file.c_str(),
                       key_file.c_str());
    server.startListen(backend_url);
}
