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
    privateKey.clear();
    publicKey.clear();
}

void Sign_cls::key_generation()
{
    cout << "----Key Generation----" << endl;
    RSA *privateRSA = genPrivateRSA();
    char *pubKey = genPublicRSA(privateRSA);
    publicKey = pubKey;
    cout << "PRIKEY and PUBKEY are made" << endl;
    cout << "public Key = " << endl
         << publicKey << endl;
    cout << "private key = " << endl
         << privateKey;

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
    privateKey = pem_pkey;
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
    return publicKey;
}

string Sign_cls::getPrivateKey() const
{
    return privateKey;
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

    *base64Text = (*bufferPtr).data;
}

void Sign_cls::handleErrors()
{
    ERR_print_errors_fp(stderr);
    abort();
}

void Sign_cls::sign_hash(queue<string> &HASH_QUEUE)
{
    queue<string> sign(HASH_QUEUE);

    cout << "----Signing Hash by private Key" << endl
         << endl;

    char *ch = new char[signedHashBufSize];

    while (!sign.empty())
    {
        string signed_hash = signMessage(privateKey, sign.front());

        memset(ch, 0, sizeof(char) * signedHashBufSize);
        strcpy(ch, signed_hash.c_str());

        hash_signed_queue.push(signed_hash);

        cout << "signed_hash : " << signed_hash << endl;

        sign.pop();
    }

    delete[] ch;
    cout << "    Signed Hash made: " << hash_signed_queue.size() << endl;
}

void Sign_cls::clearQueue()
{
    while (!hash_signed_queue.empty())
        hash_signed_queue.pop();
}
