#include "../../include/server/FASTXOXTServer.hpp"

FASTXOXTServer::FASTXOXTServer(char *dbpath, char *xsetPath) {
    this->tau = new DB(std::string(dbpath));
    this->XSet = new DB(std::string(xsetPath));
    this->ctools = new CryptoppTools();

    if (!this->tau || !this->XSet)
        throw new FASTXOXTException("couldn't open database and/or Xset");

    // Load p
    std::ifstream pFile(P_FILENAME);

    if (pFile && pFile.is_open()) {
        pFile >> this->p;
        pFile.close();
    } else {
        if (this->tau)
            delete this->tau;
        if (this->ctools)
            delete this->ctools;
        if (this->XSet)
            delete this->XSet;
        throw new FASTXOXTException("p file not found (filename: " + std::string(P_FILENAME) + ")");
    }
}

FASTXOXTServer::~FASTXOXTServer() {
    if (this->tau)
        delete this->tau;
    if (this->ctools)
        delete this->ctools;
    if (this->XSet)
        delete this->XSet;
}

bool FASTXOXTServer::update(std::string u, std::string e, std::string xtag, std::string y, bool op) {
    std::cout << "Storing T[" << u << "] = [" << e << "|" << y << "]" << std::endl;
    bool retTau, retXset;

    retTau = this->tau->put(u, e + "|" + y);

    if (op) {
        std::cout << "Adding [" << xtag << "] to XSet" << std::endl;
        retXset = this->XSet->put(xtag, "");
    } else {
        std::cout << "Removing [" << xtag << "] from XSet" << std::endl;
        retXset = this->XSet->del(xtag);
    }
    return retTau & retXset;
}

std::vector<std::string> FASTXOXTServer::initSearch(std::string tw, std::string stc, int c) { // FAST single keyword search
    std::vector<std::string> t, delta;
    std::string u, e, tmp, ind, ki, sti, ey, y;
    char op;

    std::cout << "Search first keyword" << std::endl;
    sti = this->ctools->decodeBase64(stc);
    tw = this->ctools->decodeBase64(tw);
    for (int i = c; i > 0; i--) { // i = c to 1
        u = this->ctools->H1(tw + sti);
        try {
            ey = this->tau->get(this->ctools->encodeBase64(u));
        } catch (FASTXOXTException *e) {
            std::cout << "ERROR: Couldn't find T[" << this->ctools->encodeBase64(u) << "]" << std::endl;
            t.clear();
            return t;
        }
        e = this->ctools->decodeBase64(ey.substr(0, ey.find_first_of('|')));
        y = ey.substr(ey.find_first_of('|') + 1);
        tmp = this->ctools->my_xor(e, this->ctools->H2(tw + sti));
        ind = tmp.substr(0, tmp.find_first_of('|') - 1);
        op = tmp.substr(tmp.find_first_of('|') - 1, tmp.find_first_of('|')).c_str()[0]; // 1 = add, 0 = del (1 byte)
        ki = this->ctools->decodeBase64(tmp.substr(tmp.find_first_of('|') + 1));
        if (op == '0')
            delta.push_back(ind);
        else {
            if (std::find(delta.begin(), delta.end(), ind) != delta.end())
                std::remove(delta.begin(), delta.end(), ind);
            else
                t.insert(t.begin(), ind + "|" + y);
        }
        sti = this->ctools->pseudorandomPermutationInv(ki, sti);
    }
    return t;
}

std::string FASTXOXTServer::skimSearch(std::vector<std::string> xtokenList, std::string tc) {
    std::vector<std::string> ret;
    std::string e, y, XSetSearch;

    e = tc.substr(0, tc.find('|'));
    y = this->ctools->decodeBase64(tc.substr(tc.find('|') + 1));
    for (auto xtoken = xtokenList.begin(); xtoken != xtokenList.end(); xtoken++) {
        XSetSearch = this->ctools->encodeBase64(this->ctools->power(this->ctools->decodeBase64(*xtoken), y, this->p));
        try {
            this->XSet->get(XSetSearch);
        } catch (FASTXOXTException *e) {
            return "";
        }
    }
    return e;
}