#include "../../include/server/TCPServer.hpp"

int main(int ac, char **av) {
    TCPServer *server = nullptr;

    // Error handling
    if (ac != 5) {
        std::cout << "Usage: " << av[0] << " <IP> <PORT> <TAU_PATH> <XSET_PATH>" << std::endl;
        return 84;
    }

    try {
        server = new TCPServer(av[1], std::atoi(av[2]), av[3], av[4]);
        server->selectLoop();
    } catch(FASTXOXTException *e) {
        std::cerr << "Error: " << e->what() << std::endl;
    }
    if (server)
        delete server;
    return 0;
}