#include "http_server.h"
#include <yaml-cpp/yaml.h>

using namespace std;

int main(int argc, char** argv) {
    YAML::Node config = YAML::LoadFile("config.yaml");

    const string max_connections = config["max connections"].as<string>();
    const string port = config["port"].as<string>();
    const string public_dir = config["public folder"].as<string>();
    const string backend_url = config["backend url"].as<string>();

    HTTPserver server(max_connections, port, public_dir);
    server.startListen(backend_url);
}

