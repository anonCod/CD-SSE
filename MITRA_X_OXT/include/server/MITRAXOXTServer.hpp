#ifndef MITRAXOXT_SERVER
# define MITRAXOXT_SERVER

#include "DB.hpp"
#include "cryptoppTools.hpp"

#define P_FILENAME "p"

class MITRAXOXTServer {
    public:
        MITRAXOXTServer(char *dbPath, char *xsetPath);
        ~MITRAXOXTServer();
        bool update(std::string addr, std::string val, std::string xtag, std::string y, bool op);
        std::vector<std::string> initSearch(std::vector<std::string> TList);
        std::string skimSearch(std::vector<std::string> xtokenList, std::string tc);
    private:
        DB *DictW = nullptr;
        DB *XSet = nullptr;
        CryptoppTools *ctools = nullptr;
        std::string p;
};

#endif /* MITRAXOXT_SERVER */