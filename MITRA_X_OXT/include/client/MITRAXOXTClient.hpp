#ifndef MITRAXOXT_CLIENT
# define MITRAXOXT_CLIENT

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


class MITRAXOXTClient {
    public:
        MITRAXOXTClient(char *ip, int port, const char *FileCnt = "./FileCnt");
        ~MITRAXOXTClient();

        void update();
        void search();
        void menu();
    private:
        TCPClient *client = nullptr;
        DB *FileCnt = nullptr;
        CryptoppTools *ctools = nullptr;
        int nu;
        std::string K;
        std::string Ki;
        std::string Kz;
        std::string Kx;
        std::string p;
        std::string q;
        std::string g;
};

#endif /* MITRAXOXT_CLIENT */