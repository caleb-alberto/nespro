#include "tcpServer.h"
using namespace std;

TCPserver::TCPserver(string port) {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port.c_str(), &hints, &res); // NOLINT

    int server = startServer();

    if (server == 1) {
        cerr << "Socket was unable to be created\n";
        exit(1);
    }
    else if (server == -1) {
        cerr << "Socket was unable to bind\n";
        exit(1);
    }
    else {
        cout << "Server listening on Port: " << port << endl;
    }
}

TCPserver::~TCPserver() {
    freeaddrinfo(res);
}

int TCPserver::startServer() {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) { return 1; }
    return ::bind(sockfd, res->ai_addr, res->ai_addrlen);
}

void TCPserver::startListen(string index) {
    if (listen(sockfd, 20) == -1) {
        cerr << "Unable to listen\n";
        exit(1);
    }
    cout << "Socket is listening\n";

    while(true) {
        addr_size = sizeof(client_addr);
        client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

        if (client_sockfd == -1) {
            cerr << "Unable to accept\n";
            exit(1);
        }

        sockaddr_in* client_in = (sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
        cout << "Client connected from IP: " << client_ip << endl;

        req_str = recvReq(client_sockfd);
        Request req_msg = parseReq(req_str);

        response = buildRes(req_msg, index);
        res_len = response.size();

        for (int total_sent = 0; total_sent < res_len; ) {
            int bytes_sent = send(client_sockfd,
                                  response.substr(total_sent).c_str(),
                                  res_len - total_sent,
                                  0);

            if (bytes_sent == -1) {
                cerr << "Unable to send\n";
                exit(1);
            }
            total_sent += bytes_sent;
        }

        req_str = "";
        close(client_sockfd);
    }
}

string TCPserver::recvReq(const int socket) {
    const int buf_size = 1024;
    char buf[buf_size];

    int bytes_recv;

    while (true) {
        bytes_recv = recv(socket, buf, buf_size, 0);

        if (bytes_recv == -1) {
            cerr << "Unable to recieve message\n";
            break;
        }
        else if (bytes_recv == 0) {
            cout << "Client disconnected\n";
            break;
        }

        buf[bytes_recv] = '\0';
        req_str.append(buf);

        if (req_str.find("\r\n\r\n") != string::npos) {
            break;
        }
    }
    return req_str;
}

Request TCPserver::parseReq(string req) {
    Request temp;

    if (req.size() < 1)
        return temp;

    istringstream init_stream(req);
    string token;
    vector<string> tokens;

    while (getline(init_stream, token, '\n'))
        tokens.push_back(token);

    istringstream startline_stream(tokens[0]);
    getline(startline_stream, temp.method, ' ');
    getline(startline_stream, temp.path, ' ');
    getline(startline_stream, temp.version, '\r');
    temp.version.erase(temp.version.find_last_not_of(" \t") + 1);

    if (temp.path.at(0) != '/') {
        try {
            int first = temp.path.find('/');
            int second = temp.path.find('/', first+1);
            temp.path = temp.path.substr(temp.path.find('/', second+1));
        }
        catch (out_of_range) {} // just ignore it
    }

    for (int i = 1; i != tokens.size(); i++) {
        istringstream header_stream(tokens[i]);
        string header_key;
        string header_value;

        getline(header_stream, header_key, ':');
        getline(header_stream, header_value, '\r');
        header_value.erase(0, header_value.find_first_not_of(" \t"));
        header_value.erase(header_value.find_last_not_of(" \t") + 1);

        temp.header_map[header_key] = header_value;
    }

    return temp;
}

string TCPserver::buildRes(const Request & msg, string index) {
    string res_msg;
    time_t t = time(nullptr);
    tm* gmt = gmtime(&t);
    ostringstream oss;
    oss << put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
    string time = oss.str();

    ifstream idx_stream(index);
    string static_file;
    string line;

    while (getline(idx_stream, line)) {
        static_file += line;
    }

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
                "Content-Length: "
                + to_string(static_file.size()) +
                "\r\n"
                "Connection: close\r\n"
                "\r\n"
                + static_file;
        else if (msg.method == "HEAD")
            res_msg = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Date: " + time + "\r\n"
                "Content-Length: "
                + to_string(static_file.size()) +
                "\r\n"
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
