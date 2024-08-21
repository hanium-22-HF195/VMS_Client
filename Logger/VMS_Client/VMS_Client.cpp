#include "VMS_Client.h"
#include <iostream>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

VMS_Client::VMS_Client(const string& ip, int port) : server_ip(ip), server_port(to_string(port)) {
    init_libcurl();  
}

VMS_Client::~VMS_Client() {
    curl_global_cleanup();  
}

void VMS_Client::init_libcurl() {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        cerr << "curl_global_init() failed: " << curl_easy_strerror(res) << endl;
        exit(-1); 
    }
}

void VMS_Client::send_pubkey_to_server(const string& publicKey) {
    CURL *curl = curl_easy_init();
    if(curl) {
        string url = "http://" + server_ip + ":" + server_port + "/publicKey";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, publicKey.c_str());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain"); // text/plain 헤더 사용
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Response from server: " << response_string << endl;
        }

        curl_easy_cleanup(curl);
    } else {
        cerr << "Failed to initialize CURL" << endl;
    }
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}
