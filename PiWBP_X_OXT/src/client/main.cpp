#include "../../include/client/PiWBPXOXTClient.hpp"

int main(int ac, char **av) {
    PiWBPXOXTClient *client;

    // Error handling
    if (ac != 3 && ac != 4) {
        std::cout << "Usage: " << av[0] << " <IP> <PORT> [W_PATH]" << std::endl;
        return 84;
    }
    
    // initialisation TCP client and PiWBPXOXT protocol
    try {
        if (ac == 3)
            client = new PiWBPXOXTClient(av[1], std::atoi(av[2]));
        else
            client = new PiWBPXOXTClient(av[1], std::atoi(av[2]), av[3]);
        client->menu();
    } catch(PiWBPXOXTException *e) {
        std::cout << "Error: " << e->what() << std::endl;
    }
    return 0;
}
