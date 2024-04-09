#include "../../include/client/TCPClient.hpp"

TCPClient::TCPClient(char *ip, int port) {
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockfd == -1)
        throw new PiWBPXOXTException("Couldn't create new socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        throw new PiWBPXOXTException("Connection to the server failed");
}

TCPClient::~TCPClient() {
    if (this->sockfd != -1)
        close(this->sockfd);
}

bool TCPClient::send(std::string data) {
    std::size_t n = 0;

    if (data[data.length() - 1] != '\n')
        data.append("\n");
    std::cout << "Client Sending : [" << data.substr(0, data.size() - 1) << "]" << std::endl;
    while (n < data.size()) {
        n = write(this->sockfd, data.c_str(), data.size());
        data.substr(n);
    }
    return true;
}

std::string TCPClient::receive() {
    std::string out = "";
    static std::string buf = "";
    int n;

    while (buf.find('\n') == std::string::npos) {
        out.append(buf);
        buf.clear();
        buf.resize(READ_LENGTH);
        n = read(this->sockfd, buf.data(), READ_LENGTH);
        if (n == 0)
            throw new PiWBPXOXTException("Got disconnected from the server");
    }
    out.append(buf.substr(0, buf.find('\n')));
    if (buf[buf.find('\n') + 1] == '\0')
        buf.clear();
    else
        buf = buf.substr(buf.find('\n') + 1);
    return out;
}

void TCPClient::disconnect() {
    this->send("DISCONNECT");
    close(this->sockfd);
}