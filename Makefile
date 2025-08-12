server: server.cpp https_server.cpp http_server.cpp
	clang++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib -Wall -o server server.cpp https_server.cpp http_server.cpp -lcurl -lcrypto -lssl

http: server.cpp  http_server.cpp
	clang++ -std=c++17 -Wall -o server server.cpp http_server.cpp -lcurl

clean: server
	rm server
