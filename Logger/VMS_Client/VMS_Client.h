#ifndef VMS_CLIENT_H
#define VMS_CLIENT_H

#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <queue>

using namespace std;

class VMS_Client_cls {
private:
    string server_ip;
    string server_port;

    void init_libcurl();
    string create_metadata(const string& mk_root_hash, const string& sign_hash, const string& cid);

public:
    VMS_Client_cls(const std::string& ip, int port);
    ~VMS_Client_cls();

    void send_pubkey_to_server(const string& publicKey);
    //void send_image(queue<string>& CIDQueue);
    void send_image(queue<string>& CIDQueue, queue<string>& MKRootQueue, queue<string>& SignHashQueue);
};

#endif // VMS_CLIENT_H
