#include "cryptoppTools.hpp"

CryptoppTools::CryptoppTools(int nu) {
    try { 
        CryptoPP::AutoSeededRandomPool prng;      
        std::string content, rkey;

        this->iv = CryptoPP::SecByteBlock(CryptoPP::AES::BLOCKSIZE);
        
        std::ifstream ifile(KS_FILENAME);
        if (ifile && ifile.is_open()) {
            ifile >> content;
            if (content[content.size() - 1] == '\n')
                content.pop_back();
            nu = std::atoi(content.substr(0, content.find("|")).c_str());
            rkey = this->decodeHex(content.substr(content.find("|") + 1));
            this->key = CryptoPP::SecByteBlock(nu/8);
            memcpy(this->key, rkey.c_str(), rkey.size());
            this->Ks = rkey;
        } else {
            this->key = CryptoPP::SecByteBlock(nu/8);
            prng.GenerateBlock(this->key, this->key.size());
            this->hexEncoder.Attach(new CryptoPP::StringSink(this->Ks));
            this->hexEncoder.Put(this->key, this->key.size());
            this->hexEncoder.MessageEnd();
            std::ofstream ofile(KS_FILENAME);
            ofile << nu << "|" << this->Ks;
            ofile.close();
            this->Ks = this->decodeHex(this->Ks);
        }

        this->nu = nu;
        memset(this->iv, 0, this->iv.size());// Empty IV, to encrypt with Kc1

        this->hexEncoder.Attach(new CryptoPP::FileSink(std::cout));
        std::cout << "key: ";
        this->hexEncoder.Put(this->key, this->key.size());
        this->hexEncoder.MessageEnd();
        std::cout << std::endl;

        std::cout << "iv: ";
        this->hexEncoder.Put(this->iv, this->iv.size());
        this->hexEncoder.MessageEnd();
        std::cout << std::endl;

        encrypt.SetKeyWithIV(this->key, this->key.size(), this->iv);

        // Setup of Diffie-Hellman
        std::string ps, gs, qs;
        std::ifstream pFile(P_FILENAME);
        std::ifstream qFile(Q_FILENAME);
        std::ifstream gFile(G_FILENAME);

        // p
        if (pFile && pFile.is_open()) {
            pFile >> ps;
            if (ps[ps.size() - 1] == '\n')
                ps.pop_back();
        }
        pFile.close();

        // q
        if (qFile && qFile.is_open()) {
            qFile >> qs;
            if (qs[qs.size() - 1] == '\n')
                qs.pop_back();
        }
        qFile.close();

        // g
        if (gFile && gFile.is_open()) {
            gFile >> gs;
            if (gs[gs.size() - 1] == '\n')
                gs.pop_back();
        }
        gFile.close();

        if (!ps.empty() && !qs.empty() && !gs.empty()) { // recover p and g
            CryptoPP::Integer pi(ps.c_str());
            CryptoPP::Integer qi(qs.c_str());
            CryptoPP::Integer gi(gs.c_str());
            this->p = pi;
            this->q = qi;
            this->g = gi;
        } else { // generate new p and g
            pg.Generate(1, prng, this->nu, this->nu - 1);
            this->p = pg.Prime();
            this->q = pg.SubPrime();
            this->g = pg.Generator();
            std::ofstream pFileOut(P_FILENAME);
            std::ofstream qFileOut(Q_FILENAME);
            std::ofstream gFileOut(G_FILENAME);

            // Store p
            pFileOut << this->getP();
            pFileOut.close();

            // Store q
            qFileOut << this->getQ();
            qFileOut.close();

            // Store g
            gFileOut << this->getGenerator();
            gFileOut.close();
        }

    } catch (std::exception &e) {
        throw new EnronException(std::string("Key Generation: ").append(e.what()));
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

std::string CryptoppTools::pseudorandomFunction(std::string ks, std::string h) {
    std::string cipher;

    this->setEncKey(ks);
    if (h.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(h, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(h, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string CryptoppTools::Fp(std::string ks, std::string h) {
    this->setEncKey(ks);
    std::string cipher;

    if (h.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(h, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(h, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher)));
    CryptoPP::Integer cipherInt((CryptoPP::byte *)cipher.data(), cipher.size());
    return integerToString(cipherInt % this->p);
}

std::string CryptoppTools::pseudorandomPermutation(std::string kc1, std::string stc) {
    std::string cipher;

    this->setEncKey(kc1);
    if (stc.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(stc, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(stc, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string CryptoppTools::generateRandomNumber(int size) {
    std::string out;;
    CryptoPP::byte digest[size/8];

    CryptoPP::OS_GenerateRandomBlock(false, digest, size/8);
    return CryptoppTools::byteToString(digest, size/8);
}

std::string CryptoppTools::genRandNbLowerThanP() {
    std::string out;
    ssize_t size = this->p.BitCount()/8 - 1; // malus 1 to always have number lower than p

    CryptoPP::byte digest[size];
    CryptoPP::OS_GenerateRandomBlock(false, digest, size);
    CryptoPP::Integer ret(digest, size);
    return integerToString(ret);
}

std::string CryptoppTools::mul(std::string a, std::string b, std::string modstr) {
    CryptoPP::Integer ia(a.c_str());
    CryptoPP::Integer ib(b.c_str());
    CryptoPP::Integer mod(modstr.c_str());
    CryptoPP::Integer result;

    result = (ia * ib) % mod;
    return integerToString(result);
}

std::string CryptoppTools::mul(std::string a, std::string b) {
    CryptoPP::Integer ia(a.c_str());
    CryptoPP::Integer ib(b.c_str());
    CryptoPP::Integer result;

    result = ia * ib;
    return integerToString(result);
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

std::string CryptoppTools::invMod(std::string nbstr, std::string modstr) {
    CryptoPP::Integer nb(nbstr.c_str());
    CryptoPP::Integer mod(modstr.c_str());

    return integerToString(nb.InverseMod(mod));
}

std::string CryptoppTools::my_xor(std::string a, std::string b) { // a "encrypted" with b
    std::string result = "";
    std::string out;
    bool a_done = false;
    int idxa = 0;
    int idxb = -1;

    if (a.size() == 0) {
        std::cout << "Error: A=0" << std::endl;
        throw new EnronException("Mod 0");
    }
    if (b.size() == 0) {
        std::cout << "Error: B=0" << std::endl;
        throw new EnronException("Mod 0");
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

int CryptoppTools::getNu() {
    return this->nu;
}
std::string CryptoppTools::getKs() {
    return this->Ks;
}

std::string CryptoppTools::getP() {
    return integerToString(this->p);
}

std::string CryptoppTools::getQ() {
    return integerToString(this->q);
}

std::string CryptoppTools::getGenerator() {
    return integerToString(this->g);
}

std::string CryptoppTools::integerToString(CryptoPP::Integer i) {
    std::ostringstream stream;
    std::string ret;

    stream << i;
    ret = stream.str();
    ret.pop_back();
    return ret;
}

bool CryptoppTools::keyFileExist() {
    std::ifstream ifile(KS_FILENAME);

    if (ifile) {
        std::cout << "Retrieving key from file (" << KS_FILENAME << ")" << std::endl;
        return true;
    } else {
        std::cout << "Key file not found (" << KS_FILENAME << "), generating new key" << std::endl;
        return false;
    }
}

std::string CryptoppTools::enc(std::string Ke, std::string ind) {
    this->setEncKey(Ke);
    std::string cipher;

    if (ind.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(ind, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(ind, true, new CryptoPP::StreamTransformationFilter(this->encrypt, new CryptoPP::StringSink(cipher)));
    return cipher;
}

std::string CryptoppTools::dec(std::string Ke, std::string e) {
    this->setDecKey(Ke);
    std::string cipher;

    if (e.size() % CryptoPP::AES::BLOCKSIZE == 0)
        CryptoPP::StringSource s(e, true, new CryptoPP::StreamTransformationFilter(this->decrypt, new CryptoPP::StringSink(cipher), CryptoPP::BlockPaddingSchemeDef::NO_PADDING));
    else
        CryptoPP::StringSource s(e, true, new CryptoPP::StreamTransformationFilter(this->decrypt, new CryptoPP::StringSink(cipher)));
    return cipher;
}

void CryptoppTools::setEncKey(std::string keyValue) {
    if(CryptoPP::AES::DEFAULT_KEYLENGTH < keyValue.size()) // Just a security, but is (hopefully) never called
        keyValue = keyValue.substr(0, CryptoPP::AES::DEFAULT_KEYLENGTH);
    else if(CryptoPP::AES::DEFAULT_KEYLENGTH > keyValue.size()) // Just a security, but is (hopefully) never called
        keyValue += std::string(CryptoPP::AES::DEFAULT_KEYLENGTH - keyValue.size(), '*'); // pad

    memcpy(this->key, keyValue.c_str(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    encrypt.SetKeyWithIV(this->key, this->key.size(), this->iv);
}

void CryptoppTools::setDecKey(std::string keyValue) {
    if(CryptoPP::AES::DEFAULT_KEYLENGTH < keyValue.size()) // Just a security, but is (hopefully) never called
        keyValue = keyValue.substr(0, CryptoPP::AES::DEFAULT_KEYLENGTH);
    else if(CryptoPP::AES::DEFAULT_KEYLENGTH > keyValue.size()) // Just a security, but is (hopefully) never called
        keyValue += std::string(CryptoPP::AES::DEFAULT_KEYLENGTH - keyValue.size(), '*'); // pad

    memcpy(this->key, keyValue.c_str(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    decrypt.SetKeyWithIV(this->key, this->key.size(), this->iv);
}

void CryptoppTools::showKeyLimits() {
    std::cout << "key length (default): " << CryptoPP::AES::DEFAULT_KEYLENGTH << " bytes (" << CryptoPP::AES::DEFAULT_KEYLENGTH * 8 << " bits)" << std::endl;
    std::cout << "key length (min): " << CryptoPP::AES::MIN_KEYLENGTH << " bytes (" << CryptoPP::AES::MIN_KEYLENGTH * 8 << " bits)" << std::endl;
    std::cout << "key length (max): " << CryptoPP::AES::MAX_KEYLENGTH << " bytes (" << CryptoPP::AES::MAX_KEYLENGTH * 8 << " bits)" << std::endl;
}

std::string CryptoppTools::byteToString(CryptoPP::byte *digest, int lenth) {
    std::string s;
    int i = -1;

    while (++i < lenth)
        s += (char)digest[i];
    return s;
}