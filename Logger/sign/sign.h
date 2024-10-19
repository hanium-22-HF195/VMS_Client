#ifndef SIGN_H
#define SIGN_H

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <openssl/rsa.h>
#include "../MK_Tree/MK_Tree.h"

using namespace std;

struct HashPair {
    string hash;
    string sign_hash;
};

class Sign_cls {
public:
    Sign_cls(int bufSize);
    ~Sign_cls();

    string getPublicKey() const;
    string getPrivateKey() const;
    char* signMessage(string privateKey, string plainText);
    void sign_hash(queue<string>& HASH_QUEUE, mutex& hash_queue_mtx);

    void start_sign_hash_thread(MK_Tree_cls& mk_tree_inst);
    void sign_hash_task(MK_Tree_cls& mk_tree_inst);

    queue<string>& getSignHashQueue() { return hash_signed_queue; }
    queue<HashPair>& getHashPairQueue() { return HashPair_queue; }
    mutex& getSignHashQueueMutex() { return hash_signed_queue_mtx; }
private:
    string m_privateKey;
    string m_publicKey;
    queue<string> hash_signed_queue;
    queue<HashPair> HashPair_queue;
    int signedHashBufSize;

    thread sign_hash_thread;
    mutex hash_signed_queue_mtx;

    RSA* genPrivateRSA();
    char* genPublicRSA(RSA *rsa);
    void key_generation(); 
    RSA* createPrivateRSA(string key);
    bool RSASign(RSA* rsa, const unsigned char* Msg, size_t MsgLen, unsigned char** EncMsg, size_t* MsgLenEnc);
    void Base64Encode(const unsigned char* buffer, size_t length, char** base64Text);
    void handleErrors();

    
};

#endif // SIGN_H
