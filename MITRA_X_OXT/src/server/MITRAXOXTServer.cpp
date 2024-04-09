#include "../../include/server/MITRAXOXTServer.hpp"

MITRAXOXTServer::MITRAXOXTServer(char *dbPath, char *xsetPath) {
    this->DictW = new DB(std::string(dbPath));
    this->XSet = new DB(std::string(xsetPath));
    this->ctools = new CryptoppTools();

    // Load p
    std::ifstream pFile(P_FILENAME);

    if (pFile && pFile.is_open()) {
        pFile >> this->p;
        pFile.close();
    } else {
        if (this->DictW)
            delete this->DictW;
        if (this->ctools)
            delete this->ctools;
        if (this->XSet)
            delete this->XSet;
        throw new MITRAXOXTException("p file not found (filename: " + std::string(P_FILENAME) + ")");
    }
}

MITRAXOXTServer::~MITRAXOXTServer() {
    if (this->DictW)
        delete this->DictW;
    if (this->XSet)
        delete this->XSet;
    if (this->ctools)
        delete this->ctools;
}

bool MITRAXOXTServer::update(std::string addr, std::string val, std::string xtag, std::string y, bool op) {
    std::cout << "Storing DictW[" << addr << "] = [" << val << "|" << y << "]" << std::endl;
    bool retXset;
    bool retT = this->DictW->put(addr, val + "|" + y);

    if (op) {
        std::cout << "Adding [" << xtag << "] to XSet" << std::endl;
        retXset = this->XSet->put(xtag, "");
    } else { // Not backward private, need to be replaced by a backward private solution
        std::cout << "Removing [" << xtag << "] from XSet" << std::endl;
        retXset = this->XSet->del(xtag);
    }
    return retT & retXset;
}

std::vector<std::string> MITRAXOXTServer::initSearch(std::vector<std::string> TList) {
    std::vector<std::string> Fw;

    for (auto it = TList.begin(); it != TList.end(); it++) {
        try {
            Fw.push_back(this->DictW->get(*it));
        } catch (MITRAXOXTException *e) {
            std::cerr << "Error: not found DictW[" + *it + "]" << std::endl;
            Fw.clear();
            return Fw;
        }
    }
    return Fw;
}

std::string MITRAXOXTServer::skimSearch(std::vector<std::string> xtokenList, std::string tc) {
    std::vector<std::string> ret;
    std::string e, y, XSetSearch;

    e = tc.substr(0, tc.find('|'));
    y = this->ctools->decodeBase64(tc.substr(tc.find('|') + 1));
    for (auto xtoken = xtokenList.begin(); xtoken != xtokenList.end(); xtoken++) {
        XSetSearch = this->ctools->encodeBase64(this->ctools->power(this->ctools->decodeBase64(*xtoken), y, this->p));
        try {
            this->XSet->get(XSetSearch);
        } catch (MITRAXOXTException *e) {
            return "";
        }
    }
    return e;
}