#ifndef SIGN_H
#define SIGN_H

#include <string>
#include <queue>
#include <openssl/rsa.h>

using namespace std;

class Sign {
private:
    string privateKey;
    string publicKey;
    queue<string> hash_signed_queue;
    int signedHashBufSize;

    RSA* genPrivateRSA();
    char* genPublicRSA(RSA *rsa);
    void key_generation(); 
    RSA* createPrivateRSA(std::string key);
    bool RSASign(RSA* rsa, const unsigned char* Msg, size_t MsgLen, unsigned char** EncMsg, size_t* MsgLenEnc);
    void Base64Encode(const unsigned char* buffer, size_t length, char** base64Text);
    void handleErrors();

public:
    Sign(int bufSize);
    ~Sign();

    string getPublicKey() const;
    string getPrivateKey() const;
    char* signMessage(std::string privateKey, std::string plainText);
    void sign_hash(queue<string>& HASH_QUEUE);

    queue<string>& getHashQueue() { return hash_signed_queue; }
};

#endif // SIGN_H
