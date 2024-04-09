#ifndef CRYPTOPPTOOLS
# define CRYPTOPPTOOLS

#include <iostream>
#include <string>

#include "cryptopp/cryptlib.h"
#include "cryptopp/rijndael.h"
#include "cryptopp/modes.h"
#include "cryptopp/files.h"
#include "cryptopp/osrng.h"
#include "cryptopp/hex.h"
#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/nbtheory.h"

#include "exception.hpp"

#define KG_FILENAME "Kg"
#define P_FILENAME "p"
#define G_FILENAME "g"
#define Q_FILENAME "q"

class CryptoppTools {
    public:
        CryptoppTools(int nu);
        ~CryptoppTools();
        std::string h(std::string in);
        std::string H1(std::string in);
        std::string H2(std::string in);
        std::string pseudorandomFunction(std::string Ks, std::string h);
        std::string Fp(std::string Ks, std::string w);
        std::string Gtag(std::string kc1, std::string stc);
        std::string GtagInv(std::string kc1, std::string stc);
        std::string generateRandomNumber(int size);
        std::string genRandNbLowerThanP();
        std::string my_xor(std::string a, std::string b);
        std::string enc(std::string Ke, std::string ind);
        std::string dec(std::string Ke, std::string e);
        std::string encodeHex(std::string s);
        std::string decodeHex(std::string s);
        std::string encodeBase64(std::string s);
        std::string decodeBase64(std::string s);
        std::string mul(std::string a, std::string b, std::string mod);
        std::string mul(std::string a, std::string b);
        std::string power(std::string nb, std::string exp, std::string mod);
        std::string invMod(std::string nb, std::string mod);
        int getNu();
        std::string getKg();
        std::string getP();
        std::string getQ();
        std::string getGenerator();
    
        static bool keyFileExist();
        static void showKeyLimits();
    private:
        void setKeyEncrypt(std::string keyValue);
        void setKeyDecrypt(std::string keyValue);
    
        static std::string byteToString(CryptoPP::byte *b, int c);
        static std::string integerToString(CryptoPP::Integer i);

        // var
        CryptoPP::SecByteBlock key;
        CryptoPP::SecByteBlock iv; // empty iv
        
        // tools
        CryptoPP::HexEncoder hexEncoder;
        CryptoPP::HexDecoder hexDecoder;
        CryptoPP::Base64Encoder b64Encoder;
        CryptoPP::Base64Decoder b64Decoder;
        CryptoPP::SHA256 hash;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encrypt;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decrypt;
        CryptoPP::RandomNumberGenerator rng;
        CryptoPP::PrimeAndGenerator pg;

        int nu = 0;
        std::string Kg;
        CryptoPP::Integer p;
        CryptoPP::Integer q;
        CryptoPP::Integer g;
};

#endif /* CRYPTOPPTOOLS */