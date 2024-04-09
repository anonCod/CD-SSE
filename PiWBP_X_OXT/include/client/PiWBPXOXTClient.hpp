#ifndef PiWBPXOXT_CLIENT
# define PiWBPXOXT_CLIENT

#include <iostream>
#include <string>

#include "TCPClient.hpp"
#include "DB.hpp"
#include "cryptoppTools.hpp"

#define KT_FILENAME "Kt"
#define KD_FILENAME "Kd"
#define KI_FILENAME "Ki"
#define KZ_FILENAME "Kz"
#define KX_FILENAME "Kx"


class PiWBPXOXTClient {
    public:
        PiWBPXOXTClient(char *ip, int port, const char *w = "./W");
        ~PiWBPXOXTClient();

        void update();
        void search();
        void menu();
    private:
        TCPClient *client = nullptr;
        DB *W = nullptr;
        CryptoppTools *ctools = nullptr;
        int nu;
        std::string Kt;
        std::string Kd;
        std::string Kg;
        std::string Ki;
        std::string Kz;
        std::string Kx;
        std::string p;
        std::string q;
        std::string g;
};

#endif /* PiWBPXOXT_CLIENT */