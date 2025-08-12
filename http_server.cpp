#include "http_server.h"
using namespace std;

HTTPserver::HTTPserver(string port, string dir) {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port.c_str(), &hints, &res);
    curl_global_init(CURL_GLOBAL_ALL);

    endpoints = parseStatDir(dir);
    int server = startServer();

    if (server == 1) {
        cerr << "Socket was unable to be created\n";
        exit(1);
    }
    else if (server == -1) {
        cerr << "Socket was unable to bind\n";
        exit(1);
    }
    else
        cout << "Server listening on Port: " << port << endl;
}

HTTPserver::~HTTPserver() {
    freeaddrinfo(res);
    curl_global_cleanup();
}

int HTTPserver::startServer() {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
        return 1;
    return ::bind(sockfd, res->ai_addr, res->ai_addrlen);

}

void HTTPserver::acceptConnection(const int socket) {
    addr_size = sizeof(client_addr);
    client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

    if (client_sockfd < 1) {
        cerr << "Unable to accept\n";
        exit(1);
    }
}

void HTTPserver::closeConnection(const int client) {
    close(client);
}

void HTTPserver::startListen(string backend_url) {
    if (listen(sockfd, 20) == -1) {
        cerr << "Unable to listen\n";
        exit(1);
    }
    cout << "Socket is listening\n";

    while(true) {
        acceptConnection(sockfd);

        sockaddr_in* client_in = (sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
        cout << "Client connected from IP: " << client_ip << endl;

        req_str = recvReq();
        Request req_msg = parseReq(req_str);


        if (!req_msg.header_map.count("Host"))
            sendResponse("HTTP 1.1 requests must include the 'Host:' header.");
        else if (endpoints.count(req_msg.path))
            buildRes(req_msg, endpoints[req_msg.path]);
        else if (req_msg.path.at(req_msg.path.size()-1) == '/'
                 && endpoints.count(req_msg.path + "index.html"))
            buildRes(req_msg, endpoints[req_msg.path + "index.html"]);
        else
            sendResponse(forwardResponse(req_msg, backend_url));

        req_str = "";
        closeConnection(client_sockfd);
    }
}

ssize_t HTTPserver::recvClient(char* buf, size_t size) {
    return recv(client_sockfd, buf, size, 0);
}

string HTTPserver::recvReq() {
    const int buf_size = 1024;
    char buf[buf_size];

    ssize_t bytes_recv;

    while (true) {
        bytes_recv = recvClient(buf, buf_size);

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

        if (bytes_recv < buf_size)
            break;
    }
    return req_str;
}

ssize_t HTTPserver::writeClient(const char* buf, const size_t size) {
    return send(client_sockfd, buf, size, 0);
}

void HTTPserver::sendResponse(string response) {
    if (response.find("Transfer-Encoding: chunked") != string::npos) {
        int start = response.find("\r\n\r\n") + 4;
        int end = response.size();

        stringstream stream;
        stream << hex << end - start;
        string chunked_size = stream.str() + "\r\n";
        string ending = "\r\n0\r\n\r\n";

        response.insert(start, chunked_size);
        response.insert(response.size(), ending);
    }
    res_len = response.size();

    size_t total_sent = 0;

    while (total_sent < res_len) {
        ssize_t bytes_sent = writeClient(
                              response.substr(total_sent).c_str(),
                              res_len - total_sent);

        if (bytes_sent == -1) {
            cerr << "Unable to send\n";
            exit(1);
        }
        total_sent += bytes_sent;
    }
}

Request HTTPserver::parseReq(string req) {
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

    if (req.find("\r\n\r\n") != string::npos)
        temp.body = req.substr(req.find("\r\n\r\n") + 4);

    return temp;
}

unordered_map<string, string> HTTPserver::parseStatDir(string dir) {
    unordered_map<string, string> paths;

    if (filesystem::exists(dir)) {
        for (auto path : std::filesystem::directory_iterator(dir)) {
            string cpath = path.path();
            if (filesystem::is_directory(cpath)) {
                for (const auto& elem : parseStatDir(cpath))
                    paths.insert(elem);
            }
            else {
                paths[cpath.substr(cpath.find('/'))] = cpath;
            }
        }
    }
    else {
        cerr << "Directory not found\n";
        exit(1);
    }

    return paths;
}

void HTTPserver::buildRes(const Request & msg, string req_path) {
    string res_msg;
    string form;
    time_t t = time(nullptr);
    tm* gmt = gmtime(&t);
    ostringstream oss;
    oss << put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
    string time = oss.str();

    size_t ext_loc = req_path.find('.') + 1;
    string type = req_path.substr(ext_loc);

    if (type == "png" || type == "jpeg" || type == "jpg" || type == "ico")
        form = "image/";
    else
        form = "text/";

    if (type == "js")
        type = "javascript";
    else if (type == "ico")
        type = "x-icon";

    ifstream f(req_path.c_str(), ios::in|ios::binary|ios::ate);
    streampos file_size = f.tellg();
    char* static_file = new char[file_size];
    f.seekg(0, ios::beg);
    f.read(static_file, file_size);
    f.close();

    res_msg = "HTTP/1.1 200 OK\r\n"
        "Content-Type: " + form + type + "\r\n"
        "Date: " + time + "\r\n"
        "Content-Length: "
        + to_string(file_size) +
        "\r\n"
        "Connection: close\r\n"
        "\r\n";

    if (form == "text/") {
        if (msg.method == "GET")
            res_msg += string(static_file);

        sendResponse(string(res_msg));
    }
    else {
        writeClient(res_msg.c_str(), res_msg.length());
        size_t total_sent = 0;

        while (total_sent < res_len) {
            ssize_t bytes_sent = writeClient(static_file, file_size);

            if (bytes_sent == -1) {
                cerr << "Unable to send\n";
                exit(1);
            }
            total_sent += bytes_sent;
        }
    }
}

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t real_size = size * nmemb;
    std::string *response = static_cast<std::string*>(userdata);
    response->append(ptr, real_size);

    return real_size;
}

string HTTPserver::forwardResponse(Request dynamic_req, string backend_url) {
    CURL* handle = curl_easy_init();
    string response;
    string full_url = backend_url + dynamic_req.path;

    curl_easy_setopt(handle, CURLOPT_URL, full_url.c_str());

    if (dynamic_req.method == "POST") {
        curl_easy_setopt(handle, CURLOPT_POST, 1);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, dynamic_req.body.c_str());
        curl_easy_setopt(handle,
                         CURLOPT_POSTFIELDSIZE,
                         (long)dynamic_req.body.size());
    }
    else if (dynamic_req.method == "GET") {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
    }

    struct curl_slist *headers = nullptr;
    for (const auto& item: dynamic_req.header_map)
        curl_slist_append(headers, (item.first + ": " + item.second).c_str());

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(handle, CURLOPT_HEADER, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&response);

    curl_easy_perform(handle);
    curl_easy_cleanup(handle);
    curl_slist_free_all(headers);

    return response;
}
