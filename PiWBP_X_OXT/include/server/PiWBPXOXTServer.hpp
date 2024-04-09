#ifndef PiWBPXOXT_SERVER
# define PiWBPXOXT_SERVER

#include "DB.hpp"
#include "cryptoppTools.hpp"

#define P_FILENAME "p"

class PiWBPXOXTServer {
    public:
        PiWBPXOXTServer(char *TPath, char *DPath, char *xsetPath);
        ~PiWBPXOXTServer();
        bool update(std::string u, std::string e, std::string xtag, std::string y, bool op);
        std::vector<std::string> initSearch(std::string labelw, std::string kw, int cw);
        std::string skimSearch(std::vector<std::string> xtokenList, std::string tc);
    private:
        DB *T = nullptr;
        DB *D = nullptr;
        DB *XSet = nullptr;
        CryptoppTools *ctools = nullptr;
        std::string p;
};

#endif /* PiWBPXOXT_SERVER */