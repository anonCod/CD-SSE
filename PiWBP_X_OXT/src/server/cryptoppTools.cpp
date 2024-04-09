#include "../../include/server/cryptoppTools.hpp"

CryptoppTools::CryptoppTools() {
    try {
        CryptoPP::AutoSeededRandomPool prng;        
        
        this->hexEncoder.Attach(new CryptoPP::FileSink(std::cout));
        this->iv = CryptoPP::SecByteBlock(CryptoPP::AES::BLOCKSIZE);
        memset(this->iv, 0, this->iv.size());// Empty IV, to encrypt with Kc1

        // Empty IV
        std::cout << "iv: ";
        this->hexEncoder.Put(this->iv, this->iv.size());
        this->hexEncoder.MessageEnd();
        std::cout << std::endl;
    } catch (std::exception &e) {
        throw new PiWBPXOXTException(std::string("Key Generation: ").append(e.what()));
    }
}

CryptoppTools::~CryptoppTools() {

}

std::string CryptoppTools::h(std::string in) {
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256 a;

    a.Update((const CryptoPP::byte*)in.data(), in.size());
    a.Final(&digest[0]);

    return CryptoppTools::byteToString(digest, sizeof(digest));
}

std::string CryptoppTools::H1(std::string in) {
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256 a;

    a.Update((const CryptoPP::byte*)in.data(), in.size());
    a.Final(&digest[0]);

    return CryptoppTools::byteToString(digest, sizeof(digest));
}

std::string CryptoppTools::H2(std::string in) {
    CryptoPP::byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256 a;

    a.Update((const CryptoPP::byte*)in.data(), in.size());
    a.Final(&digest[0]);

    return CryptoppTools::byteToString(digest, sizeof(digest));
}

std::string CryptoppTools::pseudorandomPermutationInv(std::string kc1, std::string stc) {
    this->setKey(kc1);
    std::string cipher;

    if (stc.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(stc, true, new CryptoPP::StreamTransformationFilter(this->decrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(stc, true, new CryptoPP::StreamTransformationFilter(this->decrypt, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string CryptoppTools::my_xor(std::string a, std::string b) { // a "encrypted" with b
    std::string result = "";
    std::string out;
    bool a_done = false;
    int idxa = 0;
    int idxb = -1;

    if (a.size() == 0) {
        std::cout << "Error: A=0" << std::endl;
        throw new PiWBPXOXTException("Mod 0");
    }
    if (b.size() == 0) {
        std::cout << "Error: B=0" << std::endl;
        throw new PiWBPXOXTException("Mod 0");
    }
    while (!a_done) {
        result += char((int)a[idxa] ^ (int)b[++idxb % b.size()]);
        ++idxa;
        if (idxa % a.size() == 0) {
            idxa = 0;
            a_done = true;
        }
    }
    return result;
}

std::string CryptoppTools::encodeHex(std::string s) {
    std::string ss;

    this->hexEncoder.Attach(new CryptoPP::StringSink(ss));
    this->hexEncoder.Put((CryptoPP::byte *)s.c_str(), s.size());
    this->hexEncoder.MessageEnd();
    return ss;
}

std::string CryptoppTools::decodeHex(std::string s) {
    std::string ss;

    this->hexDecoder.Attach(new CryptoPP::StringSink(ss));
    this->hexDecoder.Put((CryptoPP::byte *)s.c_str(), s.size());
    this->hexDecoder.MessageEnd();
    return ss;
}

std::string CryptoppTools::encodeBase64(std::string s) {
    std::string ss;

    this->b64Encoder.Attach(new CryptoPP::StringSink(ss));
    this->b64Encoder.Put((CryptoPP::byte *)s.c_str(), s.size());
    this->b64Encoder.MessageEnd();

    if (ss[ss.size() - 1] == '\n')
        ss.pop_back();
    return ss;
}

std::string CryptoppTools::decodeBase64(std::string s) {
    std::string ss;

    this->b64Decoder.Attach(new CryptoPP::StringSink(ss));
    this->b64Decoder.Put((CryptoPP::byte *)s.c_str(), s.size());
    this->b64Decoder.MessageEnd();

    return ss;
}

void CryptoppTools::setKey(std::string keyValue) {
    CryptoPP::SecByteBlock key = CryptoPP::SecByteBlock(CryptoPP::AES::DEFAULT_KEYLENGTH);

    // Just a security, should never be called
    if(CryptoPP::AES::DEFAULT_KEYLENGTH < keyValue.size())
        keyValue = keyValue.substr(0, CryptoPP::AES::DEFAULT_KEYLENGTH); // chop if too long
    else if(CryptoPP::AES::DEFAULT_KEYLENGTH > keyValue.size())
        keyValue += std::string(CryptoPP::AES::DEFAULT_KEYLENGTH - keyValue.size(), '*'); // pad
    memcpy(key, keyValue.c_str(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    this->decrypt.SetKeyWithIV(key, key.size(), this->iv);
}

std::string CryptoppTools::byteToString(CryptoPP::byte *digest, int length) {
    std::string s;
    int i = -1;

    while (++i < length)
        s += (char)digest[i];
    return s;
}

std::string CryptoppTools::integerToString(CryptoPP::Integer i) {
    std::ostringstream stream;
    std::string ret;

    stream << i;
    ret = stream.str();
    ret.pop_back();
    return ret;
}

std::string CryptoppTools::power(std::string nbstr, std::string expstr, std::string modstr) {
    CryptoPP::Integer nb(nbstr.c_str());
    CryptoPP::Integer exp(expstr.c_str());
    CryptoPP::Integer mod(modstr.c_str());
    CryptoPP::Integer ret = 1;

    while (exp > 0) {
        if (exp % 2) {
            ret = (nb * ret) % mod;
            exp = exp - 1;
        } else {
            nb = (nb * nb) % mod;
            exp /= 2;
        }
    }
    return integerToString(ret);
}