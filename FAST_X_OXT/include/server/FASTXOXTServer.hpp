#ifndef FASTXOXT_SERVER
# define FASTXOXT_SERVER

#include "DB.hpp"
#include "cryptoppTools.hpp"

#define P_FILENAME "p"

class FASTXOXTServer {
    public:
        FASTXOXTServer(char *dbpath, char *xsetPath);
        ~FASTXOXTServer();
        bool update(std::string u, std::string e, std::string xtag, std::string y, bool op);
        std::vector<std::string> initSearch(std::string tw, std::string stc, int c);
        std::string skimSearch(std::vector<std::string> xtokenList, std::string tc);
//        std::vector<std::string> search(std::string tw, std::string stc, int c, std::vector<std::string> xtrapList);
    private:
        DB *tau = nullptr;
        DB *XSet = nullptr;
        CryptoppTools *ctools = nullptr;
        std::string p;
};

#endif /* FASTXOXT_SERVER */