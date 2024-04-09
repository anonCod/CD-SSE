#ifndef TCPCLIENT
# define TCPCLIENT

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <ostream>

#include "exception.hpp"

#define READ_LENGTH 64

class TCPClient {
    public:
        TCPClient(char *ip, int port);
        ~TCPClient();
        bool send(std::string data);
        std::string receive();
        void disconnect();
    private:
        char *ip;
        int port;
        int sockfd = -1;
};

#endif /* TCPCLIENT */