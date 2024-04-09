#include "../../include/client/FASTXOXTClient.hpp"

int main(int ac, char **av) {
    FASTXOXTClient *client = nullptr;

    // Error handling
    if (ac != 3 && ac != 4) {
        std::cout << "Usage: " << av[0] << " <IP> <PORT> [SIGMA_PATH]" << std::endl;
        return 84;
    }
    
    // initialisation TCP client and FASTXOXT protocol
    try {
        if (ac == 3)
            client = new FASTXOXTClient(av[1], std::atoi(av[2]));
        else
            client = new FASTXOXTClient(av[1], std::atoi(av[2]), av[3]);
        client->menu();
    } catch(FASTXOXTException *e) {
        std::cout << "Error: " << e->what() << std::endl;
        return 84;
    }
    if (client)
        delete client;
    return 0;
}
