#include "../../include/client/MITRAXOXTClient.hpp"
#include <time.h>

MITRAXOXTClient::MITRAXOXTClient(char *ip, int port, const char *fileCnt /* default is "./FileCnt" */) {
    
    std::cout << "MITRAXOXT Client Initialization Started" << std::endl;
    if (!CryptoppTools::keyFileExist()) {
        CryptoppTools::showKeyLimits();
        std::cout << "Security Parameter: ";
        std::cin >> this->nu;
        if (this->nu <= 0)
            throw new MITRAXOXTException("Invalid Security Parameter");
    }

    this->ctools = new CryptoppTools(this->nu);
    this->nu = this->ctools->getNu();
    this->K = this->ctools->getK();
    this->p = this->ctools->getP();
    this->q = this->ctools->getQ();
    this->g = this->ctools->getGenerator();

    // Retreive Ki or generate it
    std::ifstream iFileKi(KI_FILENAME);
    if (iFileKi) {
        std::cout << "Retrieving Ki from file: " << KI_FILENAME << std::endl;
        iFileKi >> this->Ki;
        iFileKi.close();
    } else {
        std::cout << "Generating new " << KI_FILENAME << std::endl;
        this->Ki = this->ctools->genRandNbLowerThanP();
        std::ofstream KiFile(KI_FILENAME);
        KiFile << this->Ki;
        KiFile.close();
    }

    // Retreive Kz or generate it
    std::ifstream iFileKz(KZ_FILENAME);
    if (iFileKz) {
        std::cout << "Retrieving Kz from file: " << KZ_FILENAME << std::endl;
        iFileKz >> this->Kz;
        iFileKz.close();
    } else {
        std::cout << "Generating new " << KZ_FILENAME << std::endl;
        this->Kz = this->ctools->generateRandomNumber(this->nu);
        std::ofstream KzFile(KZ_FILENAME);
        KzFile << this->Kz;
        KzFile.close();
    }

    // Retreive Kx or generate it
    std::ifstream iFileKx(KX_FILENAME);
    if (iFileKx) {
        std::cout << "Retrieving Kx from file: " << KX_FILENAME << std::endl;
        iFileKx >> this->Kx;
        iFileKx.close();
    } else {
        std::cout << "Generating new " << KX_FILENAME << std::endl;
        this->Kx = this->ctools->generateRandomNumber(this->nu);
        std::ofstream KxFile(KX_FILENAME);
        KxFile << this->Kx;
        KxFile.close();
    }

    this->FileCnt = new DB(std::string(fileCnt));
    std::cout << "MITRAXOXT Client Initialization Done" << std::endl;

    try {
        this->client = new TCPClient(ip, port);
    } catch (MITRAXOXTException *e) {
        if (this->ctools)
            delete this->ctools;
        if (this->FileCnt)
            delete this->FileCnt;
        throw new MITRAXOXTException(e->what());
    }
}

MITRAXOXTClient::~MITRAXOXTClient() {
    if (this->client)
        delete this->client;
    if (this->ctools)
        delete this->ctools;
    if (this->FileCnt)
        delete this->FileCnt;
}

void MITRAXOXTClient::update() {
    // Get Args
    std::string w, op, ind;
    double elapsed;

    elapsed = 0;
    
    for(int i = 1; i <= 72252961; i++){
        // std::cout << "Index: ";
        // std::cin >> ind;
        // std::cout << "Keyword: ";
        // std::cin >> w;
        // std::cout << "Operation (add or del): ";
        // std::cin >> op;
        // while (op != "add" and op != "del") {
        //     std::cout << "Invalid operation" << std::endl;
        //     std::cout << "Operation (add or del): ";
        //     std::cin >> op;
        // }
        ind = std::to_string(i);
        w = "avi";
        op = "add";
        char opcode = '1';
        int fileCntw;
        // char opcode = ((op == "add") ? '1' : '0');
        std::string addr, val, tmp, cOXT;
        std::string request = "U";

        clock_t start = clock();
        try {
            tmp = this->FileCnt->get(w);
            fileCntw = std::atoi(tmp.substr(0, tmp.find_first_of('|')).c_str());
            cOXT = tmp.substr(tmp.find_first_of('|') + 1);
        } catch (MITRAXOXTException *e) {
            fileCntw = 0;
            cOXT = "0";
        }
        fileCntw++;
        this->FileCnt->put(w, std::to_string(fileCntw) + "|" + std::to_string(std::atoi(cOXT.c_str()) + 1));
        addr = this->ctools->G(this->K, w + std::to_string(fileCntw) + "0" );
        val = this->ctools->my_xor(ind + opcode, this->ctools->G(this->K, w + std::to_string(fileCntw) + "1"));

        // OXT part
        std::string xind, z, y, xtag;
        xind = this->ctools->Fp(this->Ki, ind);
        z = this->ctools->Fp(this->Kz, w + cOXT);
        y = this->ctools->encodeBase64(this->ctools->mul(xind, this->ctools->invMod(z, this->q), this->q));

        xtag = ((op == "add") ? "1" : "0"); // To indicate to the server if xtag must be added or removed from XSet    
        xtag += this->ctools->encodeBase64(this->ctools->power(this->g, this->ctools->mul(this->ctools->Fp(this->Kx, w), xind), this->p));
        if (!this->client->send(request.append(this->ctools->encodeBase64(addr)).append("|").append(this->ctools->encodeBase64(val)).append("|").append(xtag).append("|").append(y)))
            throw new MITRAXOXTException("Unable to send data to server");

        // Handling server's answer
        std::string answer = this->client->receive();
        clock_t end = clock();
        elapsed += (double(end - start)/CLOCKS_PER_SEC);
        std::cout << "Server answer: " << answer << std::endl;
    }
    std::cout << "Time measured in seconds:" << elapsed << std::endl;
}

void MITRAXOXTClient::search() {
    std::string w, choice;
    std::vector<std::string> possibleChoices, keywords, t;
    std::vector<std::string> positiveChoice = {"y", "yes", "Yes", "YES"};
    std::vector<std::string> negativeChoice = {"n", "no", "No", "NO"};
    bool allKeywordSet = false;
    
    possibleChoices = positiveChoice;
    possibleChoices.insert(possibleChoices.end(), negativeChoice.begin(), negativeChoice.end());
    while (!allKeywordSet) {
        std::cout << "Keyword: ";
        std::cin >> w;
        try {
            this->FileCnt->get(w); // verify that the keyword exist
            keywords.push_back(w);
            choice = "";
            while (std::find(possibleChoices.begin(), possibleChoices.end(), choice) == possibleChoices.end()) {
                std::cout << "Add another keyword ? [yes/no]: ";
                std::cin >> choice;
            }
            if (std::find(negativeChoice.begin(), negativeChoice.end(), choice) != negativeChoice.end())
                allKeywordSet = true;
        } catch (MITRAXOXTException *e) {
            std::cout << "This keyword doesn't exist" << std::endl;
            return;
        }
    }
    w = keywords[0]; // least frequent keyword ? possible to find it with Sigma but may require a lot of computation

    // algo
    int fileCntw;
    std::string TList;

    try {
        fileCntw = std::atoi(this->FileCnt->get(w).c_str());
    } catch (MITRAXOXTException *e) {
        std::cout << "This keyword doesn't exist" << std::endl;
        return;
    }
    for (int i = 1; i <= fileCntw; i++)
        TList += this->ctools->encodeBase64(this->ctools->G(this->K, w + std::to_string(i) + "0")) + "|";
    if (!TList.empty())
        TList.pop_back(); // remove last '|'

    std::string request = "S";
    if (!this->client->send(request.append(TList)))
        throw new MITRAXOXTException("Unable to send data to server");
    
    // Handling server's answer
    std::string answer = this->client->receive();
    if (answer != "OK") { // first keyword search returned an empty list of index
        std::cout << "Result: No matching index." << std::endl;
        return;
    }

    // OXT Conjunctive Search part
    std::string allToken;
    int cOXT = 0;

    while (answer != "STOP") {
        allToken.clear();
        for (int i = 1; i < (int)keywords.size(); i++) {
            allToken.append(this->ctools->encodeBase64(
                this->ctools->power(
                    this->g,
                    this->ctools->mul(
                        this->ctools->Fp(this->Kz, w + std::to_string(cOXT)),
                        this->ctools->Fp(this->Kx, keywords[i]),
                        this->q),
                    this->p)
            ));
            allToken += "|";
        }

        if (!allToken.empty())
            allToken.pop_back();
        this->client->send("X" + allToken);
        answer = this->client->receive();
        std::cout << "Server's answer: [" << answer << "]" << std::endl;
        if (!answer.empty() && answer != "STOP")
            t.push_back(this->ctools->decodeBase64(answer));
        else
            t.push_back("NO_MATCH");
        ++cOXT;
    }

    // Computing matching index
    std::string tmp, ind;
    std::vector<std::string> Rw;
    std::vector<std::string>::iterator pos;
    char op;
    int j = 1;

    if (t.empty()) {
        std::cout << "Result: No matching index found." << std::endl;
    } else {
        std::cout << "Result: ";
        for (auto it = t.begin(); it != t.end(); it++) {
            // Decode result
            if (*it != "NO_MATCH") {
                tmp = this->ctools->my_xor(*it, this->ctools->G(this->K, w + std::to_string(j) + "1"));
                ind = tmp.substr(0, tmp.size() - 1);
                op = tmp[tmp.size() - 1];
                pos = std::find(Rw.begin(), Rw.end(), ind);
                if (op == '1' && pos == Rw.end())
                    Rw.push_back(ind);
                else if (op == '0' && pos != Rw.end())
                    Rw.erase(pos);
            }
            j++;
        }
        // Printing decoded result
        std::cout << "Result: ";
        for (auto it = Rw.begin(); it != Rw.end(); it++) {
            std::cout << *it;
            if (it + 1 != Rw.end())
                std::cout << ", ";
        }
        std::cout << std::endl;
    }
}

void MITRAXOXTClient::menu() {
    bool go = true;
    std::string choice;
    while (go) {
        std::cout << "Please Select an Option:" << std::endl;
        std::cout << "0 = Update" << std::endl;
        std::cout << "1 = Search" << std::endl;
        std::cout << "q = Exit" << std::endl;
        
        std::cout << "> ";
        std::cin >> choice;
        if (choice == "0") {
            this->update();
        } else if (choice == "1") {
            this->search();
        } else if (choice == "q" || choice == "") {
            go = false;
        } else {
            std::cout << "Unknown command" << std::endl;
        }
        choice = "";
    }
    this->client->disconnect();
}