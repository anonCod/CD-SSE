#ifndef TCPSERVER
# define TCPSERVER

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "exception.hpp"
#include "PiWBPXOXTServer.hpp"

#define READ_LENGTH 64

typedef struct client_s {
    int fd;
    int c = 0;
    bool isSearching = false;
    std::vector<std::string> t;
    std::chrono::milliseconds::rep timestamp;
} client_t;

class TCPServer {
    public:
        TCPServer(char *ip, int port, char *TPath, char *DPath, char *xsetPath);
        ~TCPServer();
        std::string Read(int fd);
        void Write(int fd, std::string msg);
        void disconnect(int fd);
        void handleUpdate(int fd, std::string msg);
        void handleInitSearch(client_t *client, std::string msg);
        void handleSkimSearch(client_t *client, std::string msg);
        void selectLoop();
    private:
        int maxfd;
        int sockfd = -1;
        int client = -1;
        struct sockaddr_in addrServer;
        struct sockaddr_in addrClient;
        fd_set FDRead;
        fd_set FDWrite;
        fd_set FDExcept;
        std::vector<client_t> clientList;
        std::map<int, std::string> buff;

        PiWBPXOXTServer *PiWBPXOXT = nullptr;
};

#endif /* TCPSERVER */