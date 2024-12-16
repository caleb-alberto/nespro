#include "tcpServer.h"
#include <stdexcept>

TCPserver::TCPserver(std::string port) {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port.c_str(), &hints, &res); // NOLINT

    int server = startServer();

    if (server == 1) {
        std::cerr << "Socket was unable to be created\n";
        exit(1);
    }
    else if (server == -1) {
        std::cerr << "Socket was unable to bind\n";
        exit(1);
    }
    else {
        std::cout << "Server listening on Port: " << port << std::endl;
    }
}

TCPserver::~TCPserver() {
    freeaddrinfo(res);
}

int TCPserver::startServer() {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) { return 1; }
    return bind(sockfd, res->ai_addr, res->ai_addrlen);
}

void TCPserver::startListen() {
    if (listen(sockfd, 20) == -1) {
        std::cerr << "Unable to listen\n";
        exit(1);
    }
    std::cout << "Socket is listening\n";

    while(true) {
        addr_size = sizeof(client_addr);
        client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

        if (client_sockfd == -1) {
            std::cerr << "Unable to accept\n";
            exit(1);
        }

        sockaddr_in* client_in = (sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected from IP: " << client_ip << std::endl;

        req_str = recvReq(client_sockfd);
        Request req_msg = parseReq(req_str);
        std::cout << req_msg.path << '\n';
        response = buildRes(req_msg);
        res_len = response.size();

        for (int total_sent = 0; total_sent < res_len; ) {
            int bytes_sent = send(client_sockfd,
                                  response.substr(total_sent).c_str(),
                                  res_len - total_sent,
                                  0);

            if (bytes_sent == -1) {
                std::cerr << "Unable to send\n";
                exit(1);
            }
            total_sent += bytes_sent;
        }

        req_str = "";
        close(client_sockfd);
    }
}

std::string TCPserver::recvReq(const int socket) {
    const int buf_size = 1024;
    char buf[buf_size];

    int bytes_recv;

    while (true) {
        bytes_recv = recv(socket, buf, buf_size, 0);

        if (bytes_recv == -1) {
            std::cerr << "Unable to recieve message\n";
            break;
        }
        else if (bytes_recv == 0) {
            std::cout << "Client disconnected\n";
            break;
        }

        buf[bytes_recv] = '\0';
        req_str.append(buf);

        if (req_str.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    return req_str;
}

Request TCPserver::parseReq(std::string req) {
    Request temp;

    if (req.size() < 1)
        return temp;

    std::istringstream init_stream(req);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(init_stream, token, '\n'))
        tokens.push_back(token);

    std::istringstream startline_stream(tokens[0]);
    std::getline(startline_stream, temp.method, ' ');
    std::getline(startline_stream, temp.path, ' ');
    std::getline(startline_stream, temp.version, '\r');
    temp.version.erase(temp.version.find_last_not_of(" \t") + 1);

    if (temp.path.at(0) != '/') {
        try {
            int first = temp.path.find('/');
            int second = temp.path.find('/', first+1);
            temp.path = temp.path.substr(temp.path.find('/', second+1));
        }
        catch (std::out_of_range) {}
    }

    for (int i = 1; i != tokens.size(); i++) {
        std::istringstream header_stream(tokens[i]);
        std::string header_key;
        std::string header_value;

        std::getline(header_stream, header_key, ':');
        std::getline(header_stream, header_value, '\r');
        header_value.erase(0, header_value.find_first_not_of(" \t"));
        header_value.erase(header_value.find_last_not_of(" \t") + 1);

        temp.header_map[header_key] = header_value;
    }

    return temp;
}

std::string TCPserver::buildRes(const Request & msg) {
    std::string res_msg;
    std::time_t t = std::time(nullptr);
    std::tm* gmt = std::gmtime(&t);
    std::ostringstream oss;
    oss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
    std::string time = oss.str();

    if (!msg.header_map.count("Host"))
        res_msg = "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Date: " + time + "\r\n"
            "Content-Length: 107\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body>"
            "<h2>No Host: header received</h2>"
            "HTTP 1.1 requests must include the Host: header."
            "</body></html>";

    else if (msg.path == "/") {
        if (msg.method == "GET")
            res_msg = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 103\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h2>Welcome to My Website</h2>"
                "<p>Your request was successfully processed!</p>"
                "</body></html>";
        else if (msg.method == "HEAD")
            res_msg = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 103\r\n"
                "Connection: close\r\n"
                "\r\n";
    }

    else if (msg.path == "/favicon.ico") {
        if (msg.method == "GET")
            res_msg = "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 108\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h1>404 Not Found</h1>"
                "<p>"
                "Sorry, we don't have a favicon for you at the moment."
                "</p>"
                "</body></html>";
        else if (msg.method == "HEAD")
            res_msg = "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 108\r\n"
                "Connection: close\r\n"
                "\r\n";
    }

    else {
        if (msg.method == "GET")
            res_msg = "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 77\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h1>404 Not Found</h1>"
                "<p>"
                "Invalid page requested"
                "</p>"
                "</body></html>";
        else if (msg.method == "GET")
            res_msg = "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 77\r\n"
                "Connection: close\r\n"
                "\r\n";
        else
            res_msg = "HTTP/1.1 400 Bad Request\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: 70\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h1>400 Not Found</h1>"
                "<p>"
                "Invalid request"
                "</p>"
                "</body></html>";
    }
    return res_msg;
}
