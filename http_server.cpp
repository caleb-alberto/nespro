/*
 * at end of processing log message using objcet
 * processing each connection (which happens after accept())
 * should happen in a seperate function for compatibility with
 * multithreading
 */

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

    lfd = open("server_log.txt", O_WRONLY | O_CREAT | O_APPEND);

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
    close(lfd);
}

int HTTPserver::startServer() {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
        return 1;
    return ::bind(sockfd, res->ai_addr, res->ai_addrlen);

}

void HTTPserver::acceptConnection(int& client_sockfd, Message& msg) {
    sockaddr_in client_addr;
    char client_ip[INET_ADDRSTRLEN];
    addr_size = sizeof(client_addr);

    client_sockfd = accept(sockfd, (sockaddr *)&client_addr, &addr_size);

    if (client_sockfd < 1) {
        msg.error = true;
        msg.error_msg = "Unable to accept";
        conditional = false;
    }
    else {
        sockaddr_in* client_in = (sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &client_in->sin_addr, client_ip, INET_ADDRSTRLEN);
        msg.ip = string(client_ip);
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

    vector<thread> threads;
    connections = 0;

    while(true) {
        conditional = true;
        Message msg;
        int client_sockfd;

        acceptConnection(client_sockfd, msg);

        if (conditional && connections < 20) {
            connections++;
            threads.emplace_back(&HTTPserver::performThread,
                                 this,
                                 client_sockfd,
                                 ref(msg),
                                 backend_url).detach();
        }
    }
}

void HTTPserver::performThread(int client_sockfd,
                               Message& msg,
                               string backend_url) {
    clock_t start = clock();
    bool conditional = true;

    string req_str;
    if (conditional && recvReq(client_sockfd, msg, req_str) < 1)
        conditional = false;

    if (conditional) {
        Request req_msg = parseReq(req_str);

        msg.method = req_msg.method;
        msg.path = req_msg.path;
        msg.version = req_msg.version;
        msg.user_agent = req_msg.header_map["User-Agent"];

        if (!req_msg.header_map.count("Host")) {
            sendResponse(client_sockfd,
                         "HTTP 1.1 requests must include the 'Host:' header.\n",
                         msg);
            msg.error = true;
            msg.error_msg = "HTTP 1.1 requests must include the 'Host:' header.";
        }
        else if (endpoints.count(req_msg.path)) {
            msg.host = req_msg.header_map["Host"];
            buildRes(client_sockfd,
                     req_msg.method,
                     endpoints[req_msg.path], msg);
        }
        else {
            msg.host = req_msg.header_map["Host"];
            try {
                if (req_msg.path.at(req_msg.path.size() - 1) == '/'
                     && endpoints.count(req_msg.path + "index.html"))
                    buildRes(client_sockfd,
                             req_msg.method,
                             endpoints[req_msg.path + "index.html"],
                             msg);
                else
                    sendResponse(client_sockfd,
                                 forwardResponse(req_msg, backend_url),
                                 msg);
            }
            catch (out_of_range) {}
        }
    }
    closeConnection(client_sockfd);
    clock_t end = clock();
    msg.time = (double)(end - start) / CLOCKS_PER_SEC;
    logMessage(msg);
    connections--;
}

ssize_t HTTPserver::recvClient(int client_sockfd, char* buf, size_t size) {
    return recv(client_sockfd, buf, size, 0);
}

ssize_t HTTPserver::recvReq(int client_sockfd, Message& msg, string& req_str) {
    const int buf_size = 1024;
    char buf[buf_size];

    ssize_t bytes_recv;

    while (true) {
        bytes_recv = recvClient(client_sockfd, buf, buf_size);

        if (bytes_recv == -1) {
            msg.error = true;
            msg.error_msg = "Unable to recieve message";
            break;
        }
        else if (bytes_recv == 0) {
            msg.error = true;
            msg.error_msg = "Client disconnected";
            break;
        }

        buf[bytes_recv] = '\0';
        req_str.append(buf);

        if (bytes_recv < buf_size)
            break;
    }
    msg.read = bytes_recv;
    return bytes_recv;
}

ssize_t HTTPserver::writeClient(int client_sockfd,
                                const char* buf,
                                const size_t size) {
    return send(client_sockfd, buf, size, 0);
}

void HTTPserver::sendResponse(int client_sockfd,
                              string response,
                              Message& msg) {
    if (response.find("Transfer-Encoding: chunked") != string::npos) {
        int start = response.find("\r\n\r\n") + 4;
        int end = response.size();

        ostringstream stream;
        stream << hex << end - start;
        string chunked_size = stream.str() + "\r\n";
        string ending = "\r\n0\r\n\r\n";

        response.insert(start, chunked_size);
        response.insert(response.size(), ending);
    }
    res_len = response.size();

    size_t total_sent = 0;

    while (total_sent < res_len) {
        ssize_t bytes_sent = writeClient(client_sockfd,
                                         response.substr(total_sent).c_str(),
                                         res_len - total_sent);

        if (bytes_sent == -1) {
            msg.error = true;
            msg.error_msg = "Unable to send message";
        }
        total_sent += bytes_sent;
    }
    msg.sent = total_sent;
}

void HTTPserver::logMessage(Message& msg) {
    ostringstream oss;

    time_t t = time(nullptr);
    tm* local_time = localtime(&t);
    ostringstream time_oss;
    time_oss << put_time(local_time, "[%Y-%m-%d %H:%M:%S]");

    oss << time_oss.str();

    if (msg.error) {
        oss << " [ERROR] ";

        if (msg.error_msg != "Unable to accept")
            oss << "[" << msg.ip << "]";
        oss << endl << msg.error_msg << "\n\n";
    }
    else {
        msg.user_agent.erase(msg.user_agent.find(')') + 1);

        oss << " [INFO] [" << msg.ip << "]\n";
        oss << '"' << msg.method << ' ' << msg.path << ' ' << msg.version << '"';
        oss << " [" << fixed << setprecision(5) << msg.time << "] - ";
        oss << msg.read << "rx/" << msg.sent << "tx -\n";
        oss << '"' << msg.user_agent << '"' << " - " << msg.host << " -";
        if (!msg.tls_version.empty())
            oss << ' ' << msg.tls_version;
        oss << "\n\n";
    }

    string log_msg = oss.str();
    write(lfd, log_msg.c_str(), log_msg.length());
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

    try {
        if (!temp.path.empty() && temp.path[0] != '/') {
            int first = temp.path.find('/');
            int second = temp.path.find('/', first+1);
            temp.path = temp.path.substr(temp.path.find('/', second+1));
        }
    }
    catch (out_of_range) {} // just ignore it

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

void HTTPserver::buildRes(int client_sockfd,
                          string method,
                          string req_path,
                          Message& msg) {
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
    else if (type == "pdf" || type == "json")
        form = "application/";
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
        "Server: nespro\r\n"
        "Connection: close\r\n"
        "\r\n";

    if (form == "text/") {
        if (method == "GET")
            res_msg += string(static_file);

        sendResponse(client_sockfd, string(res_msg), msg);
    }
    else {
        writeClient(client_sockfd, res_msg.c_str(), res_msg.length());
        size_t total_sent = 0;

        while (total_sent < res_len) {
            ssize_t bytes_sent = writeClient(client_sockfd,
                                             static_file,
                                             file_size);

            if (bytes_sent == -1) {
                msg.error = true;
                msg.error_msg = "Unable to send message";
            }
            total_sent += bytes_sent;
        }
        msg.sent = total_sent;
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
