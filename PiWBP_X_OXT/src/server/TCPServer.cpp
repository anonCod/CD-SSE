#include "../../include/server/TCPServer.hpp"

TCPServer::TCPServer(char *ip, int port, char *TPath, char *DPath, char *xsetPath) {
    int optval = 1;

    memset(&this->addrServer, 0, sizeof(this->addrServer));
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
        throw new PiWBPXOXTException("Couldn't create new socket");

    setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));    
    this->addrServer.sin_family = AF_INET;
    this->addrServer.sin_addr.s_addr = inet_addr(ip);
    this->addrServer.sin_port = htons(port);
    if (bind(this->sockfd, (struct sockaddr *)&this->addrServer, sizeof(this->addrServer)) != 0)
        throw new PiWBPXOXTException("Socket bind failed");
    if (listen(this->sockfd, 10) != 0)
        throw new PiWBPXOXTException("Listen failed");

    FD_ZERO(&this->FDRead);
    FD_ZERO(&this->FDWrite);
    FD_ZERO(&this->FDExcept);
    this->maxfd = this->sockfd;

    std::cout << "Initialization of PiWBPXOXT server..." << std::endl;
    this->PiWBPXOXT = new PiWBPXOXTServer(TPath, DPath, xsetPath);
    std::cout << "Done" << std::endl;
}

TCPServer::~TCPServer() {
    if (this->sockfd != -1)
        close(this->sockfd);
    if (this->client != -1)
        close(this->client);
    if (this->PiWBPXOXT)
        delete this->PiWBPXOXT;
    for (auto client = this->clientList.begin(); client != this->clientList.end(); client++) {
        this->disconnect(client->fd);
        client--;
    }
}

std::string TCPServer::Read(int fd) {
    std::string out = "";
    static std::string buf = "";
    int n;

    while (buf.find('\n') == std::string::npos) {
        out.append(buf);
        buf.clear();
        buf.resize(READ_LENGTH);
        n = read(fd, buf.data(), READ_LENGTH);
        if (n == 0) {
            std::cout << "Client " << fd << " got disconnected" << std::endl;
            return "DISCONNECT";
        }
    }
    out.append(buf.substr(0, buf.find('\n')));
    if (buf[buf.find('\n') + 1] == '\0')
        buf.clear();
    else
        buf = buf.substr(buf.find('\n') + 1);
    return out;
}

void TCPServer::Write(int fd, std::string msg) {
    std::size_t n = 0;

    while (n < msg.size()) {
        n = write(fd, msg.c_str(), msg.size());
        msg.substr(n);
    }
}

void TCPServer::disconnect(int fd) {
    close(fd);
    this->buff.erase(fd);
    for (auto client = this->clientList.begin(); client != this->clientList.end(); client++) {
        if (client->fd == fd) {
            this->clientList.erase(client);
            --client;
        }
    }
    std::cout << "Connection closed with " << fd << std::endl;
}

void TCPServer::handleUpdate(int fd, std::string msg) {
    std::string label, e, xtag, y;
    bool op;

    std::cout << "Handling Update:" << std::endl;
    label = msg.substr(0, msg.find('|'));
    msg = msg.substr(msg.find_first_of('|') + 1);
    e = msg.substr(0, msg.find('|'));
    msg = msg.substr(msg.find_first_of('|') + 1);
    op = ((msg[0] == '1') ? true : false);
    xtag = msg.substr(1, msg.find_first_of('|') - 1); // first charac is op
    y = msg.substr(msg.find_first_of('|') + 1);

    if (this->PiWBPXOXT->update(label, e, xtag, y, op))
        this->buff.insert({fd, "OK"});
    else
        this->buff.insert({fd, "KO"});
}

void TCPServer::handleInitSearch(client_t *client, std::string msg) { // TODO
    std::string answer = "";
    std::vector<std::string> TS;
    std::string label, Kw;
    int cw;

    std::cout << "Handling Search" << std::endl;
    label = msg.substr(0, msg.find_first_of('|'));
    Kw = msg.substr(msg.find_first_of('|') + 1, msg.find_last_of('|') - (msg.find_first_of('|') + 1));
    cw = std::atoi(msg.substr(msg.find_last_of('|') + 1).c_str());
    
    client->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    TS = this->PiWBPXOXT->initSearch(label, Kw, cw);

    if (TS.empty()) { // keyword not found
        this->buff.insert({client->fd, "KO"});
        client->timestamp = 0;
    } else {
        this->buff.insert({client->fd, "OK"});
        client->t = TS;
        client->c = 0;
        client->isSearching = true;
    }
}

void TCPServer::handleSkimSearch(client_t *client, std::string msg) {
    std::vector<std::string> allToken;
    std::string token, e;
    std::string ret = "";
    
    if (!client->isSearching) { // Client can't use this request if they didn't initiate a search first
        std::cout << "Error: client " << client->fd << " has no search ongoing" << std::endl;
        this->buff.insert({client->fd, "KO"});
        return;
    }

    if (client->c == (int)client->t.size()) { // Stop search when last tuple in t is reached
        this->buff.insert({client->fd, "STOP"});
        std::cout << "Search Query of client " << client->fd << " is over." << std::endl;
        auto after = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::cerr << "Search Time: " << after - client->timestamp << " ms" << std::endl;
        client->isSearching = false;
        client->c = 0;
        client->timestamp = 0;
        return;
    }

    std::cout << "Skim Search with xtokens: " << msg << std::endl;
    while (msg.find('|') != std::string::npos) {
        token = msg.substr(0, msg.find_first_of('|'));
        msg = msg.substr(msg.find_first_of('|') + 1);
        allToken.push_back(token);
    }
    if (!msg.empty())
        allToken.push_back(msg);
    this->buff.insert({client->fd, this->PiWBPXOXT->skimSearch(allToken, client->t[client->c])});
    client->c += 1;
}

void TCPServer::selectLoop() {
    struct timeval delay;
    std::string msg;
    int fd, newFD;
    socklen_t len;

    while (true) {
        delay.tv_sec = 2;
        delay.tv_usec = 0;
        FD_ZERO(&this->FDRead);
        FD_ZERO(&this->FDWrite);
        FD_ZERO(&this->FDExcept);
        FD_SET(this->sockfd, &this->FDRead);
        FD_SET(this->sockfd, &this->FDExcept);
        for (auto client = this->clientList.begin(); client != this->clientList.end(); client++) {
            FD_SET(client->fd, &this->FDRead);
            FD_SET(client->fd, &this->FDExcept);
        }
        for (auto i = this->buff.begin(); i != this->buff.end(); i++) {
            if (!FD_ISSET(i->first, &this->FDWrite))
                FD_SET(i->first, &this->FDWrite);
        }

        fd = select(this->maxfd + 1, &this->FDRead, &this->FDWrite,&this->FDExcept, &delay);
        if (fd == -1)
            throw new PiWBPXOXTException("Select Error");

        if (fd != 0) { // = not timeout
            // New connection
            if (FD_ISSET(this->sockfd, &this->FDRead)) {
                std::cout << "New connection" << std::endl;
                len = sizeof(this->addrClient);
                newFD = accept(this->sockfd, (struct sockaddr *)&this->addrClient, &len);
                client_t newClient;
                newClient.c = 0;
                newClient.isSearching = false;
                newClient.fd = newFD;
                newClient.timestamp = 0;
                std::cout << "new fd: " << newFD << std::endl;
                this->clientList.push_back(newClient);
                this->maxfd = (this->maxfd > newFD) ? this->maxfd : newFD;
            }

            // check for each client
            for (auto client = this->clientList.begin(); client != this->clientList.end(); client++) {
                // Client deconnection
                if (FD_ISSET(client->fd, &this->FDExcept)) {
                    this->disconnect(client->fd);
                    client--;
                }
                // Send to client
                if (FD_ISSET(client->fd, &this->FDWrite)) {
                    std::cout << "Sending [" << this->buff[client->fd] << "] to client " << client->fd << std::endl;
                    this->Write(client->fd, this->buff[client->fd] + "\n");
                    this->buff.erase(client->fd);
                }
                // Receive from client
                if (FD_ISSET(client->fd, &this->FDRead)) {
                    std::cout << "Receiving from client " << client->fd << std::endl;
                    msg = this->Read(client->fd);
                    if (msg[0] == 'S') { // search first keyword
                        this->handleInitSearch(&(*client), msg.substr(1));
                    } else if (msg[0] == 'X') { // skim search
                        this->handleSkimSearch(&(*client), msg.substr(1));
                    } else if (msg[0] == 'U') { // update
                        this->handleUpdate(client->fd, msg.substr(1)); // remove first (U) and last (\n) character
                    } else if (msg == "DISCONNECT") { // disconnection
                        this->disconnect(client->fd);
                        client--;
                    } else { // Unknown / corrupted message
                        std::cout << "Unknown message received from " << client->fd << ": [" << msg << "] - first charac int : ["  << (int)msg[0] << "]" <<std::endl;
                        this->buff.insert({client->fd, "KO"});
                    }
                }
            }
        }
    }
} 