#include "../../include/server/TCPServer.hpp"

int main(int ac, char **av) {

    // Error handling
    if (ac != 6) {
        std::cout << "Usage: " << av[0] << " <IP> <PORT> <T_PATH> <D_PATH> <XSET_PATH>" << std::endl;
        return 84;
    }

    try {
        TCPServer *server = new TCPServer(av[1], std::atoi(av[2]), av[3], av[4], av[5]);
        server->selectLoop();
    } catch(PiWBPXOXTException *e) {
        std::cerr << "Error: " << e->what() << std::endl;
    }
    return 0;
}