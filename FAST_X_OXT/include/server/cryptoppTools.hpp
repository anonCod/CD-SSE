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

#include "exception.hpp"

class CryptoppTools {
    public:
        CryptoppTools();
        ~CryptoppTools();
        std::string h(std::string in);
        std::string H1(std::string in);
        std::string H2(std::string in);
        std::string pseudorandomFunction(std::string k, std::string v);
        std::string pseudorandomPermutationInv(std::string kc1, std::string stc);
        std::string my_xor(std::string a, std::string b);
        std::string power(std::string nb, std::string exp, std::string mod);
        std::string encodeHex(std::string s);
        std::string decodeHex(std::string s);
        std::string encodeBase64(std::string s);
        std::string decodeBase64(std::string s);
    private:
        void setEncKey(std::string keyValue);
        void setDecKey(std::string keyValue);

        static std::string byteToString(CryptoPP::byte *b, int c);
        static std::string integerToString(CryptoPP::Integer i);

        // var
        CryptoPP::SecByteBlock iv; // empty iv

        // tools
        CryptoPP::HexEncoder hexEncoder;
        CryptoPP::HexDecoder hexDecoder;
        CryptoPP::Base64Encoder b64Encoder;
        CryptoPP::Base64Decoder b64Decoder;
        CryptoPP::SHA256 hash;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decrypt;
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encrypt;
        CryptoPP::RandomNumberGenerator rng;
};

#endif /* CRYPTOPPTOOLS */