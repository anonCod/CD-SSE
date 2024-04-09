#include "../../include/server/PiWBPXOXTServer.hpp"

PiWBPXOXTServer::PiWBPXOXTServer(char *TPath, char *DPath, char *xsetPath) {
    this->T = new DB(std::string(TPath));
    this->D = new DB(std::string(DPath));
    this->XSet = new DB(std::string(xsetPath));
    this->ctools = new CryptoppTools();

    // Load p
    std::ifstream pFile(P_FILENAME);

    if (pFile && pFile.is_open()) {
        pFile >> this->p;
        pFile.close();
    } else {
        if (this->T)
            delete this->T;
        if (this->D)
            delete this->D;
        if (this->ctools)
            delete this->ctools;
        if (this->XSet)
            delete this->XSet;
        throw new PiWBPXOXTException("p file not found (filename: " + std::string(P_FILENAME) + ")");
    }
}

PiWBPXOXTServer::~PiWBPXOXTServer() {
    if (this->T)
        delete this->T;
    if (this->D)
        delete this->D;
    if (this->XSet)
        delete this->XSet;
    if (this->ctools)
        delete this->ctools;
}

bool PiWBPXOXTServer::update(std::string label, std::string e, std::string xtag, std::string y, bool op) {
    std::cout << "Storing T[" << label << "] = [" << e << "|" << y << "]" << std::endl;
    bool retXset;
    bool retT = this->T->put(label, e + "|" + y);

    if (op) {
        std::cout << "Adding [" << xtag << "] to XSet" << std::endl;
        retXset = this->XSet->put(xtag, "");
    } else { // Not backward private, need to be replaced by a backward private solution
        std::cout << "Removing [" << xtag << "] from XSet" << std::endl;
        retXset = this->XSet->del(xtag);
    }
    return retT & retXset;
}

std::vector<std::string> PiWBPXOXTServer::initSearch(std::string labelw, std::string Kw, int cw) {
    int c;
    std::string label, e, pad, tag, value, ey, y;
    std::vector<std::string> TS;
    bool b;
    
    Kw = this->ctools->decodeBase64(Kw);
    try {
        value = this->D->get(labelw);
        if (!value.empty()) {
            while (value.find_first_of(',') != std::string::npos) {
                TS.push_back(value.substr(0, value.find_first_of(',')));
                value = value.substr(value.find_first_of(',') + 1);
            }
            TS.push_back(value);
        }
    } catch(PiWBPXOXTException *e) {
        
    }

    if (Kw != "NONE") {
        c = 0;
        while (c <= cw) {
            label = this->ctools->encodeBase64(this->ctools->H1(Kw + std::to_string(c)));
            ey = this->T->get(label);
            e = this->ctools->decodeBase64(ey.substr(0, ey.find_first_of('|')));
            y = ey.substr(ey.find_first_of('|') + 1);
            pad = this->ctools->H2(Kw + std::to_string(c));
            value = this->ctools->my_xor(e, pad);
            b = ((value[0] == '0') ? true : false);
            tag = value.substr(1);
            if (b) {
                TS.push_back(this->ctools->encodeBase64(tag) + "|" + y);
            } else {
                TS.erase(std::find(TS.begin(), TS.end(), this->ctools->encodeBase64(tag) + "|" + y));
            }
            this->T->del(label);
            c++;
        }
    }

    value = "";
    for (auto it = TS.begin(); it != TS.end(); it++)
        value.append(*it).append(",");
    if (!value.empty())
        value.pop_back();
    this->D->put(labelw, value);
    return TS;
}

std::string PiWBPXOXTServer::skimSearch(std::vector<std::string> xtokenList, std::string tc) {
    std::vector<std::string> ret;
    std::string e, y, XSetSearch;

    e = tc.substr(0, tc.find('|'));
    y = this->ctools->decodeBase64(tc.substr(tc.find('|') + 1));
    for (auto xtoken = xtokenList.begin(); xtoken != xtokenList.end(); xtoken++) {
        XSetSearch = this->ctools->encodeBase64(this->ctools->power(this->ctools->decodeBase64(*xtoken), y, this->p));
        try {
            this->XSet->get(XSetSearch);
        } catch (PiWBPXOXTException *e) {
            return "";
        }
    }
    return e;
}