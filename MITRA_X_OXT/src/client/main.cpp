#include "../../include/client/MITRAXOXTClient.hpp"

int main(int ac, char **av) {
    MITRAXOXTClient *client;

    // Error handling
    if (ac != 3 && ac != 4) {
        std::cout << "Usage: " << av[0] << " <IP> <PORT> [FILECNT_PATH]" << std::endl;
        return 84;
    }
    
    // initialisation TCP client and MITRAXOXT protocol
    try {
        if (ac == 3)
            client = new MITRAXOXTClient(av[1], std::atoi(av[2]));
        else
            client = new MITRAXOXTClient(av[1], std::atoi(av[2]), av[3]);
        client->menu();
    } catch(MITRAXOXTException *e) {
        std::cout << "Error: " << e->what() << std::endl;
    }
    return 0;
}
