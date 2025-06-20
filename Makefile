server: server.cpp http_server.cpp
	clang++ -std=c++17 -Wall -o server server.cpp http_server.cpp -lcurl

clean: server
	rm server
