#ifndef VMS_CLIENT_H
#define VMS_CLIENT_H

#include <fstream>
#include <chrono>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <queue>
#include <thread>
#include <pthread.h>
#include <mutex>
#include "../sign/sign.h"
#include "../media/Media.h"
#include "../MK_Tree/MK_Tree.h"
#include "../inference/inference.h"
#include "../matadata/matadata.h"

#include <spdlog/spdlog.h>

using namespace std;

class VMS_Client_cls {
public:
    VMS_Client_cls(const string& ip, int port);
    ~VMS_Client_cls();

    void send_pubkey_to_server(const string& publicKey);
    //void send_image(queue<string>& CIDQueue);
    void send_image(queue<matadata>& matadata_queue, mutex& matadata_mutex);

    void start_send_image_thread(queue<matadata>& matadata_queue, mutex& matadata_mutex); // t7 스레드를 시작하는 함수
    void send_image_task(queue<matadata>& matadata_queue, mutex& matadata_mutex);         // t7 스레드의 실제 작업 함수
private:
    string server_ip;
    string server_port;
    string image_uploadUrl;
    string responseData;
    string metadata;
    
    thread send_image_thread;
    
    void init_libcurl();
    string create_metadata(const string& mk_root_hash, const string& sign_hash, const string& cid, const string& result);

};

#endif // VMS_CLIENT_H
