#ifndef VMS_CLIENT_H
#define VMS_CLIENT_H

#include <string>
#include <curl/curl.h>

using namespace std;

class VMS_Client {
private:
    string server_ip;
    string server_port;

    void init_libcurl();

public:
    VMS_Client(const std::string& ip, int port);
    ~VMS_Client();

    void send_pubkey_to_server(const string& publicKey);
};

#endif // VMS_CLIENT_H
