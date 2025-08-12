server: server.cpp https_server.cpp
	clang++ -std=c++17 -I/opt/homebrew/include -L/opt/homebrew/lib -Wall -o server server.cpp https_server.cpp http_server.cpp -lcurl -lcrypto -lssl

clean: server
	rm server
