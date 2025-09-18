CXX      := clang++
CXXFLAGS := -std=c++17 -Wall
LDLIBS   := -lcurl -lyaml-cpp

UNAME_S  := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += -I/opt/homebrew/include
    LDFLAGS  += -L/opt/homebrew/lib
endif

SERVER_SRCS := ssl_server.cpp https_server.cpp http_server.cpp
HTTP_SRCS   := server.cpp http_server.cpp

all: server

server: $(SERVER_SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS) -lcrypto -lssl

http: $(HTTP_SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o server $(LDLIBS)

clean:
	rm -f server

.PHONY: all http clean
