#include "../../include/client/FASTXOXTClient.hpp"
#include <time.h>

FASTXOXTClient::FASTXOXTClient(char *ip, int port, const char *sigma /* default is "./Sigma" */) {
    
    std::cout << "FASTXOXT Client Initialization Started" << std::endl;
    if (!CryptoppTools::keyFileExist()) {
        CryptoppTools::showKeyLimits();
        std::cout << "Security Parameter: ";
        std::cin >> this->nu;
        if (this->nu <= 0)
            throw new FASTXOXTException("Invalid Security Parameter");
    }

    this->ctools = new CryptoppTools(this->nu);
    this->p = this->ctools->getP();
    this->q = this->ctools->getQ();
    this->g = this->ctools->getGenerator();
    this->nu = this->ctools->getNu();
    this->Ks = this->ctools->getKs();
    this->Sigma = new DB(std::string(sigma));

    // Retreive Kx or generate it
    std::ifstream iFileKx(KX_FILENAME);
    if (iFileKx) {
        std::cout << "Retrieving Kx from file: " << KX_FILENAME << std::endl;
        iFileKx >> this->Kx;
        iFileKx.close();
    } else {
        std::cout << "Generating new " << KX_FILENAME << std::endl;
        this->Kx = this->ctools->genRandNbLowerThanP();
        std::ofstream KxFile(KX_FILENAME);
        KxFile << this->Kx;
        KxFile.close();
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
        this->Kz = this->ctools->genRandNbLowerThanP();
        std::ofstream KzFile(KZ_FILENAME);
        KzFile << this->Kz;
        KzFile.close();
    }

    std::cout << "FASTXOXT Client Initialization Done" << std::endl;

    try {
        this->client = new TCPClient(ip, port);
    } catch (FASTXOXTException *e) {
        if (this->ctools)
            delete this->ctools;
        if (this->Sigma)
            delete this->Sigma;
        throw new FASTXOXTException(e->what());
    }
}

FASTXOXTClient::~FASTXOXTClient() {
    if (this->client)
        delete this->client;
    if (this->ctools)
        delete this->ctools;
    if (this->Sigma)
        delete this->Sigma;
}

void FASTXOXTClient::update() {
    // Get Args
    std::string w, op, ind, y, xind, z;
    char opcode;
    double elapsed;

    elapsed = 0;
    
    for(int i = 1; i <= 72252961; i++){
        // std::cout << "Index: ";
        // std::cin >> ind;
        // std::cout << "Keyword: ";
        // std::cin >> w;
        // std::cout << "Operation (add or del): ";
        // std::cin >> op;
        ind = std::to_string(i);
        w = "avi";
        op = "add";
        opcode = '1';
        while (op != "add" && op != "del") {
            std::cout << "Invalid operation" << std::endl;
            std::cout << "Operation (add or del): ";
            std::cin >> op;
        }
        // opcode = ((op == "add") ? '1' : '0'); // add=1, del=0

        clock_t start = clock();
        // Encode ind with Ke for OXT
        std::string xtagInd = ind;
        std::string Ke = this->ctools->pseudorandomFunction(this->Ks, w);
        ind = this->ctools->enc(Ke, ind);

        // Algo
        std::string tw, stc, value, kc1, stc1, c, e, u, xtag;
        std::string request = "U";

        tw = this->ctools->pseudorandomFunction(this->Ks, this->ctools->h(w));
        try {
            value = this->Sigma->get(w);
            stc = this->ctools->decodeBase64(value.substr(0, value.find('|')));
            c = value.substr(value.find('|') + 1);
        } catch (FASTXOXTException *e) {
            stc = this->ctools->generateRandomNumber(this->nu);
            c = "0";
        }
        kc1 = this->ctools->generateRandomNumber(this->nu);
        stc1 = this->ctools->pseudorandomPermutation(kc1, stc);
        this->Sigma->put(w, this->ctools->encodeBase64(stc1) + "|" + std::to_string(std::atoi(c.c_str()) + 1));
        e = this->ctools->my_xor((this->ctools->encodeBase64(ind) + opcode + "|" + this->ctools->encodeBase64(kc1)), this->ctools->H2(tw + stc1));
        u = this->ctools->H1(tw + stc1);

        // OXT part, add or remove xtag from XSet
        xind = this->ctools->Fp(this->Ki, xtagInd);
        z = this->ctools->Fp(this->Kz, w + c); // using FAST c in Sigma as OXT c value
        y = this->ctools->encodeBase64(this->ctools->mul(xind, this->ctools->invMod(z, this->q), this->q));

        // xtag = ((op == "add") ? "1" : "0"); // To indicate to the server if xtag must be added or removed from XSet    
        xtag = "1"; // To indicate to the server if xtag must be added or removed from XSet    
        xtag += this->ctools->encodeBase64(this->ctools->power(this->g, this->ctools->mul(this->ctools->Fp(this->Kx, w), xind), this->p));
        if (!this->client->send(request.append(this->ctools->encodeBase64(u)).append("|").append(this->ctools->encodeBase64(e)).append("|").append(xtag).append("|").append(y)))
            throw new FASTXOXTException("Unable to send data to server");

        // Handling server's answer
        std::string answer = this->client->receive();
        clock_t end = clock();
        elapsed += (double(end - start)/CLOCKS_PER_SEC);
        std::cout << "Server answer: " << answer << std::endl;
    }
    std::cout << "Time measured in seconds:" << elapsed << std::endl;
}

void FASTXOXTClient::search() {
    std::string w, stc, value, c, tw, choice;
    std::string request = "S";
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
            this->Sigma->get(w); // verify that the keyword exist
            keywords.push_back(w);
            choice = "";
            while (std::find(possibleChoices.begin(), possibleChoices.end(), choice) == possibleChoices.end()) {
                std::cout << "Add another keyword ? [yes/no]: ";
                std::cin >> choice;
            }
            if (std::find(negativeChoice.begin(), negativeChoice.end(), choice) != negativeChoice.end())
                allKeywordSet = true;
        } catch (FASTXOXTException *e) {
            std::cout << "This keyword doesn't exist" << std::endl;
            return;
        }
    }
    w = keywords[0]; // least frequent keyword ? possible to find it with Sigma but may require a lot of computation

    tw = this->ctools->pseudorandomFunction(this->Ks, this->ctools->h(w));
    value = this->Sigma->get(w);
    stc = value.substr(0, value.find('|'));
    c = value.substr(value.find('|') + 1);

    this->client->send("S" + this->ctools->encodeBase64(tw).append("|").append(stc).append("|").append(c));
    std::string answer = this->client->receive();

    if (answer != "OK") { // first keyword search returned an empty list of index
        std::cout << "Result: No matching index." << std::endl;
        return;
    }

    // OXT part, generate and send xtoken until server send STOP
    std::string allToken;
    int cOxt = 0;

    while (answer != "STOP") {
        allToken.clear();
        for (int i = 1; i < (int)keywords.size(); i++) {
            allToken.append(this->ctools->encodeBase64(
                this->ctools->power(
                    this->g,
                    this->ctools->mul(
                        this->ctools->Fp(this->Kz, w + std::to_string(cOxt)),
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
        ++cOxt;
    }

    std::cout << "Result: ";
    std::string Ke = this->ctools->pseudorandomFunction(this->Ks, w);
    if (t.empty())
        std::cout << "No matching index." << std::endl;
    else {
        for (auto it = t.begin(); it != t.end() - 1; it++)
            std::cout << this->ctools->dec(Ke, *it) << ", ";
        std::cout << this->ctools->dec(Ke, *(--t.end())) << std::endl;
    }
}

void FASTXOXTClient::menu() {
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