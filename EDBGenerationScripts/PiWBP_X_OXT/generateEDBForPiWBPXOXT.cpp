#include <regex>

#include "cryptoppTools.hpp"
#include "DB.hpp"

#define DEBUG_LIMIT 10000
#define SEPARATOR_LIST " \"':-_/\\\t\n*><|(){}[]*%.,;!?$£"
#define BUFF_SIZE 128

#define KX_FILENAME "Kx"
#define KI_FILENAME "Ki"
#define KZ_FILENAME "Kz"
#define KT_FILENAME "Kt"

int main(int ac, char **av) {
    if (ac != 2) {
        std::cout << "Usage: " << av[0] << " <csv_file>" << std::endl;
        return 0;
    }

    int pair = 0;
    int id = 0;

    CryptoppTools ctools(128);

    // TODO : get paramss
    DB W("./W");
    DB T("./T");
    DB XSet("./XSet");
    std::string Kg = ctools.getKg();
    int nu = ctools.getNu();
    std::string p = ctools.getP();
    std::string q = ctools.getQ();
    std::string g = ctools.getGenerator();
    std::string Kx, Kz, Ki, Kt;

    // Retreive Kt or generate it
    std::ifstream iFileKt(KT_FILENAME);
    if (iFileKt) {
        std::cout << "Retrieving Kt from file: " << KT_FILENAME << std::endl;
        iFileKt >> Kt;
        iFileKt.close();
    } else {
        std::cout << "Generating new " << KT_FILENAME << std::endl;
        Kt = ctools.generateRandomNumber(nu);
        std::ofstream KtFile(KT_FILENAME);
        KtFile << Kt;
        KtFile.close();
    }

    // Retreive Kx or generate it
    std::ifstream iFileKx(KX_FILENAME);
    if (iFileKx) {
        std::cout << "Retrieving Kx from file: " << KX_FILENAME << std::endl;
        iFileKx >> Kx;
        iFileKx.close();
    } else {
        std::cout << "Generating new " << KX_FILENAME << std::endl;
        Kx = ctools.genRandNbLowerThanP();
        std::ofstream KxFile(KX_FILENAME);
        KxFile << Kx;
        KxFile.close();
    }

    // Retreive Ki or generate it
    std::ifstream iFileKi(KI_FILENAME);
    if (iFileKi) {
        std::cout << "Retrieving Ki from file: " << KI_FILENAME << std::endl;
        iFileKi >> Ki;
        iFileKi.close();
    } else {
        std::cout << "Generating new " << KI_FILENAME << std::endl;
        Ki = ctools.genRandNbLowerThanP();
        std::ofstream KiFile(KI_FILENAME);
        KiFile << Ki;
        KiFile.close();
    }

    // Retreive Kz or generate it
    std::ifstream iFileKz(KZ_FILENAME);
    if (iFileKz) {
        std::cout << "Retrieving Kz from file: " << KZ_FILENAME << std::endl;
        iFileKz >> Kz;
        iFileKz.close();
    } else {
        std::cout << "Generating new " << KZ_FILENAME << std::endl;
        Kz = ctools.genRandNbLowerThanP();
        std::ofstream KzFile(KZ_FILENAME);
        KzFile << Kz;
        KzFile.close();
    }

    std::ifstream file(av[1]);
    std::string line;
    int debug = 0;
    char psBuffer[BUFF_SIZE];
    bool read = false;

    try {
        while (std::getline(file, line)) { // && debug < DEBUG_LIMIT
    //        std::cout << "line: [" << line << "]" << std::endl;
            // skip until start of new message
            if (line.find("X-FileName:") != std::string::npos) {
                id++;
                read = true;
                if (id % 100 == 0)
                    std::cout << id << std::endl;
            } else if (read) { // debug: && 
                //std::cout << "proc read" << std::endl;
                // read until only one '"' found
                std::string message = line;
                while (std::getline(file, line) && line.find("Message-ID:") == std::string::npos) {
                    message += line;
                    //std::cout << "line added: [" << line << "]" << std::endl;
                }
                //std::cout << "end while: [" << line << "]" << std::endl;

                // parser
                //std::cout << "message to parser: [" << message << "]" << std::endl;
                std::vector<std::string> keywords;
                std::string w = "";
                std::string separator(SEPARATOR_LIST);

                // verif if already exist in keywords

                for (auto it = message.begin(); it != message.end(); it++) {
                    if (std::find(separator.begin(), separator.end(), *it) != separator.end()) {
                        if (!w.empty() && std::find(keywords.begin(), keywords.end(), w) == keywords.end()) {
                            keywords.push_back(w);
                            //std::cout << w << std::endl;
                            pair++;

                            // FAST_X_OXT update algo
                            //   PiWBP part
                            // Algo
                            std::string ind = std::to_string(id);
                            int verw, cw;
                            std::string value, Kw, label, pad, tag, e, b, cOXT;

                            try {
                                value = W.get(w);
                                verw = std::atoi(value.substr(0, value.find_first_of('|')).c_str());
                                value = value.substr(value.find_first_of('|') + 1);
                                cw = std::atoi(value.substr(0, value.find_first_of('|')).c_str());
                                cOXT = value.substr(value.find_first_of('|') + 1);
                            } catch(EnronException *e) {
                                verw = 0;
                                cw = -1;
                                cOXT = "0";
                            }
                            cw++;
                            Kw = ctools.pseudorandomFunction(Kt, w + std::to_string(verw));
                            label = ctools.H1(Kw + std::to_string(cw));
                            pad = ctools.H2(Kw + std::to_string(cw));
                            tag = ctools.Gtag(Kg, ctools.encodeBase64(w) + "|" + ind);
                            b = '0';
                            e = ctools.my_xor(b + tag, pad);
                            W.put(w, std::to_string(verw) + "|" + std::to_string(cw) + "|" + std::to_string(std::atoi(cOXT.c_str()) + 1));


                            //   OXT part, add or remove xtag from XSet
                            std::string xind, z, y, xtag;
                            xind = ctools.Fp(Ki, ind);
                            z = ctools.Fp(Kz, w + cOXT);
                            y = ctools.encodeBase64(ctools.mul(xind, ctools.invMod(z, q), q));

                            xtag = ctools.encodeBase64(ctools.power(g, ctools.mul(ctools.Fp(Kx, w), xind), p));

                            // add to XSet and database
                            XSet.put(xtag, "");
                            T.put(ctools.encodeBase64(label), ctools.encodeBase64(e) + "|" + y);
                        }
                        w.clear();
                    } else
                        w += *it;
                }
                keywords.clear();
                line.clear();
                read = false;
            }
            //debug++;
        }
    } catch (EnronException *e) {
        std::cout << "Error: " << e->what() << std::endl;
        return 84;
    }
    file.close();
    std::cout << "id: " << id << std::endl;
    std::cout << "pair: " << pair << std::endl;
    return 0;
}