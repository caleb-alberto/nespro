CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -I/opt/homebrew/include
LDFLAGS  := -L/opt/homebrew/lib
LDLIBS   := -lcurl

SERVER_SRCS := ssl_server.cpp https_server.cpp http_server.cpp
HTTP_SRCS   := server.cpp http_server.cpp

all: server

server: $(SERVER_SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS) -lcrypto -lssl -lyaml-cpp

http: $(HTTP_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o server $(LDLIBS) -lyaml-cpp

clean:
	rm -f server

.PHONY: all http clean
