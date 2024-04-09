#ifndef FASTXOXT_CLIENT
# define FASTXOXT_CLIENT

#include <iostream>
#include <string>

#include "TCPClient.hpp"
#include "DB.hpp"
#include "cryptoppTools.hpp"

#define KX_FILENAME "Kx"
#define KI_FILENAME "Ki"
#define KZ_FILENAME "Kz"

class FASTXOXTClient {
    public:
        FASTXOXTClient(char *ip, int port, const char *sigma = "./Sigma");
        ~FASTXOXTClient();

        void update();
        void search();
        void menu();
    private:
        TCPClient *client = nullptr;
        DB *Sigma = nullptr;
        CryptoppTools *ctools = nullptr;
        int nu;
        std::string Ks;
        std::string Kx;
        std::string Ki;
        std::string Kz;
        std::string p;
        std::string q;
        std::string g;
};

#endif /* FASTXOXT_CLIENT */