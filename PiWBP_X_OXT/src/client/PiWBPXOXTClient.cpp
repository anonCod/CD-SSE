#include "../../include/client/PiWBPXOXTClient.hpp"
#include <time.h>

PiWBPXOXTClient::PiWBPXOXTClient(char *ip, int port, const char *w /* default is "./Sigma" */) {
    
    std::cout << "PiWBPXOXT Client Initialization Started" << std::endl;
    if (!CryptoppTools::keyFileExist()) {
        CryptoppTools::showKeyLimits();
        std::cout << "Security Parameter: ";
        std::cin >> this->nu;
        if (this->nu <= 0)
            throw new PiWBPXOXTException("Invalid Security Parameter");
    }

    this->ctools = new CryptoppTools(this->nu);
    this->nu = this->ctools->getNu();
    this->Kg = this->ctools->getKg();
    this->p = this->ctools->getP();
    this->q = this->ctools->getQ();
    this->g = this->ctools->getGenerator();
    
    // Retreive Kt or generate it
    std::ifstream iFileKt(KT_FILENAME);
    if (iFileKt) {
        std::cout << "Retrieving Kt from file: " << KT_FILENAME << std::endl;
        iFileKt >> this->Kt;
        iFileKt.close();
    } else {
        std::cout << "Generating new " << KT_FILENAME << std::endl;
        this->Kt = this->ctools->generateRandomNumber(this->nu);
        std::ofstream KtFile(KT_FILENAME);
        KtFile << this->Kt;
        KtFile.close();
    }
    
    // Retreive Kd or generate it
    std::ifstream iFileKd(KD_FILENAME);
    if (iFileKd) {
        std::cout << "Retrieving Kd from file: " << KD_FILENAME << std::endl;
        iFileKd >> this->Kd;
        iFileKd.close();
    } else {
        std::cout << "Generating new " << KD_FILENAME << std::endl;
        this->Kd = this->ctools->generateRandomNumber(this->nu);
        std::ofstream KdFile(KD_FILENAME);
        KdFile << this->Kd;
        KdFile.close();
    }

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

    this->W = new DB(std::string(w));
    std::cout << "PiWBPXOXT Client Initialization Done" << std::endl;

    try {
        this->client = new TCPClient(ip, port);
    } catch (PiWBPXOXTException *e) {
        if (this->ctools)
            delete this->ctools;
        if (this->W)
            delete this->W;
        throw new PiWBPXOXTException(e->what());
    }
}

PiWBPXOXTClient::~PiWBPXOXTClient() {
    if (this->client)
        delete this->client;
    if (this->ctools)
        delete this->ctools;
    if (this->W)
        delete this->W;
}

void PiWBPXOXTClient::update() {
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

        // Algo
        int verw, cw;
        std::string value, Kw, label, pad, tag, e, b, cOXT;

        clock_t start = clock();
        try {
            value = this->W->get(w);
            verw = std::atoi(value.substr(0, value.find_first_of('|')).c_str());
            value = value.substr(value.find_first_of('|') + 1);
            cw = std::atoi(value.substr(0, value.find_first_of('|')).c_str());
            cOXT = value.substr(value.find_first_of('|') + 1);
        } catch(PiWBPXOXTException *e) {
            verw = 0;
            cw = -1;
            cOXT = "0";
        }
        cw++;
        Kw = this->ctools->pseudorandomFunction(this->Kt, w + std::to_string(verw));
        label = this->ctools->H1(Kw + std::to_string(cw));
        pad = this->ctools->H2(Kw + std::to_string(cw));
        tag = this->ctools->Gtag(this->Kg, this->ctools->encodeBase64(w) + "|" + ind);
        b = ((op == "add") ? '0' : '1');
        e = this->ctools->my_xor(b + tag, pad);
        this->W->put(w, std::to_string(verw) + "|" + std::to_string(cw) + "|" + std::to_string(std::atoi(cOXT.c_str()) + 1));

        // OXT part
        std::string xind, z, y, xtag;
        xind = this->ctools->Fp(this->Ki, ind);
        z = this->ctools->Fp(this->Kz, w + cOXT);
        y = this->ctools->encodeBase64(this->ctools->mul(xind, this->ctools->invMod(z, this->q), this->q));

        xtag = ((op == "add") ? "1" : "0"); // To indicate to the server if xtag must be added or removed from XSet    
        xtag += this->ctools->encodeBase64(this->ctools->power(this->g, this->ctools->mul(this->ctools->Fp(this->Kx, w), xind), this->p));
        std::string request = "U";
        if (!this->client->send(request.append(this->ctools->encodeBase64(label)).append("|").append(this->ctools->encodeBase64(e)).append("|").append(xtag).append("|").append(y)))
            throw new PiWBPXOXTException("Unable to send data to server");

        // Handling server's answer
        std::string answer = this->client->receive();
        clock_t end = clock();
        elapsed += (double(end - start)/CLOCKS_PER_SEC);
        std::cout << "Server answer: " << answer << std::endl;
    }
    std::cout << "Time measured in seconds:" << elapsed << std::endl;
}

void PiWBPXOXTClient::search() {
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
            this->W->get(w); // verify that the keyword exist
            keywords.push_back(w);
            choice = "";
            while (std::find(possibleChoices.begin(), possibleChoices.end(), choice) == possibleChoices.end()) {
                std::cout << "Add another keyword ? [yes/no]: ";
                std::cin >> choice;
            }
            if (std::find(negativeChoice.begin(), negativeChoice.end(), choice) != negativeChoice.end())
                allKeywordSet = true;
        } catch (PiWBPXOXTException *e) {
            std::cout << "This keyword doesn't exist" << std::endl;
            return;
        }
    }
    w = keywords[0]; // least frequent keyword ? possible to find it with Sigma but may require a lot of computation

    // algo
    int verw, cw;
    std::string value, label, Kw;

    try {
        value = this->W->get(w);
    } catch(PiWBPXOXTException *e) {
        std::cout << "This keyword doesn't exist." << std::endl;
        return;
    }

    verw = std::atoi(value.substr(0, value.find_first_of('|')).c_str());
    cw = std::atoi(value.substr(value.find_first_of('|') + 1).c_str());
    label = this->ctools->pseudorandomFunction(this->Kd, w);
    if (cw != -1) {
        Kw = this->ctools->pseudorandomFunction(this->Kt, w + std::to_string(verw));
        verw++;
        this->W->put(w, std::to_string(verw) + "|-1");
    } else
        Kw = "NONE";

    std::string request = "S";
    if (!this->client->send(request.append(this->ctools->encodeBase64(label)).append("|").append(this->ctools->encodeBase64(Kw)).append("|").append(std::to_string(cw))))
        throw new PiWBPXOXTException("Unable to send data to server");
    
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
        ++cOXT;
    }

    // Computing matching index
    std::string tag, ind;

    if (t.empty()) {
        std::cout << "Result: No matching index found." << std::endl;
    } else {
        std::cout << "Result: ";
        for (auto it = t.begin(); it != t.end(); it++) {
            tag = *it;
            value = this->ctools->GtagInv(this->Kg, tag);
            w = this->ctools->decodeBase64(value.substr(0, value.find_first_of('|')));
            ind = value.substr(value.find_first_of('|') + 1);
            if (it + 1 != t.end())
                std::cout << ind << ", ";
            else
                std::cout << ind << std::endl;
        }
    }
}

void PiWBPXOXTClient::menu() {
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