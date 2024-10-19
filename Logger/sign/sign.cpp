#include <iostream>
#include <fstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cstring>

#include "sign.h"

using namespace std;

Sign_cls::Sign_cls(int bufSize) : signedHashBufSize(bufSize)
{
    key_generation();
}

Sign_cls::~Sign_cls()
{
    m_privateKey.clear();
    m_publicKey.clear();
}

void Sign_cls::key_generation()
{
    cout << "----Key Generation----" << endl;
    RSA *privateRSA = genPrivateRSA();
    char *pubKey = genPublicRSA(privateRSA);
    m_publicKey = pubKey;
    cout << "PRIKEY and PUBKEY are made" << endl;
    cout << "public Key = " << endl
         << m_publicKey << endl;
    cout << "private key = " << endl
         << m_privateKey;

    free(pubKey);
    RSA_free(privateRSA);
}

RSA *Sign_cls::genPrivateRSA()
{
    RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);
    int pem_pkey_size = BIO_pending(bio);
    char *pem_pkey = (char *)calloc((pem_pkey_size) + 1, 1);
    BIO_read(bio, pem_pkey, pem_pkey_size);
    m_privateKey = pem_pkey;
    BIO_free_all(bio);
    return rsa;
}

char *Sign_cls::genPublicRSA(RSA *rsa)
{
    BIO *bp_public = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bp_public, rsa);
    int pub_pkey_size = BIO_pending(bp_public);
    char *pub_pkey = (char *)calloc((pub_pkey_size) + 1, 1);
    BIO_read(bp_public, pub_pkey, pub_pkey_size);
    BIO_free_all(bp_public);
    return pub_pkey;
}

string Sign_cls::getPublicKey() const
{
    return m_publicKey;
}

string Sign_cls::getPrivateKey() const
{
    return m_privateKey;
}

char *Sign_cls::signMessage(string privateKey, string plainText)
{
    RSA *privateRSA = createPrivateRSA(privateKey);
    unsigned char *encMessage;
    char *base64Text;
    size_t encMessageLength;

    // sign plaintext by privateKey to encMessage
    /*** KYH */
    RSASign(privateRSA, (unsigned char *)plainText.c_str(), plainText.length(), &encMessage, &encMessageLength);

    // encode encMessage to base64Text
    Base64Encode(encMessage, encMessageLength, &base64Text);

    // *** KYH
    // check encMessage and base64Text
    // encMessage is notchanged
    
    free(encMessage);
    return base64Text;
}

RSA *Sign_cls::createPrivateRSA(string key)
{
    RSA *rsa = NULL;
    const char *c_string = key.c_str();
    BIO *keybio = BIO_new_mem_buf((void *)c_string, -1);
    if (keybio == NULL)
    {
        return 0;
    }
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

    // add code handle memory leak
    BIO_free(keybio);

    return rsa;
}

bool Sign_cls::RSASign(RSA *rsa, const unsigned char *Msg, size_t MsgLen, unsigned char **EncMsg, size_t *MsgLenEnc)
{
    EVP_MD_CTX *m_RSASignCtx = EVP_MD_CTX_create();
    EVP_PKEY *priKey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(priKey, rsa);

    if (EVP_DigestSignInit(m_RSASignCtx, NULL, EVP_sha256(), NULL, priKey) <= 0)
    {
        return false;
    }

    // link error
    if (EVP_DigestSignUpdate(m_RSASignCtx, Msg, MsgLen) <= 0)
    {
        return false;
    }

    // ** RSASign
    if (EVP_DigestSignFinal(m_RSASignCtx, NULL, MsgLenEnc) <= 0)
    {
        return false;
    }

    *EncMsg = (unsigned char *)malloc(*MsgLenEnc);
    if (EVP_DigestSignFinal(m_RSASignCtx, *EncMsg, MsgLenEnc) <= 0)
    {
        return false;
    }

    // add code handle memory leak
    EVP_PKEY_free(priKey);
    EVP_MD_CTX_free(m_RSASignCtx);
    return true;
}

void Sign_cls::Base64Encode(const unsigned char *buffer, size_t length, char **base64Text)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    *base64Text = strdup((*bufferPtr).data);
}

void Sign_cls::handleErrors()
{
    ERR_print_errors_fp(stderr);
    abort();
}

void Sign_cls::sign_hash(queue<string>& HASH_QUEUE, mutex& hash_queue_mtx)
{
    HashPair m_HashPair;
    string m_hash;
    hash_queue_mtx.lock();
    m_hash = HASH_QUEUE.front();
    HASH_QUEUE.pop();
    hash_queue_mtx.unlock();

    //cout << "----Signing Hash by private Key" << endl << endl;

    char *ch = new char[signedHashBufSize];
    string signed_hash = signMessage(m_privateKey, m_hash);
        
    memset(ch, 0, sizeof(char) * signedHashBufSize);
    strcpy(ch, signed_hash.c_str());

    m_HashPair.hash = m_hash;
    m_HashPair.sign_hash = signed_hash;

    hash_signed_queue_mtx.lock();
    //hash_signed_queue.push(signed_hash);
    HashPair_queue.push(m_HashPair);
    hash_signed_queue_mtx.unlock();

    //cout << "signed_hash : " << signed_hash << endl;
    //cout << "Successfully make signed_hash" << endl;

    delete[] ch;
}


void Sign_cls::sign_hash_task(MK_Tree_cls& mk_tree_inst) {
    auto last_print_time = chrono::steady_clock::now();
    while (true) {
        if(!mk_tree_inst.getHashQueue().empty()){
            sign_hash(mk_tree_inst.getHashQueue(), mk_tree_inst.getHashQueueMutex());
        }
        auto current_time = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(current_time - last_print_time).count();
        if (elapsed >= 1) {
            cout << "    HashPair_queue size : " << HashPair_queue.size() << endl;
            last_print_time = current_time;
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Sign_cls::start_sign_hash_thread(MK_Tree_cls& mk_tree_inst) {
    sign_hash_thread = thread(&Sign_cls::sign_hash_task, this, ref(mk_tree_inst));
}