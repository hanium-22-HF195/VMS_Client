#ifndef VMS_CLIENT_H
#define VMS_CLIENT_H

#include <string>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <queue>
#include <thread>
#include <mutex>
#include "../sign/sign.h"
#include "../media/Media.h"
#include "../MK_Tree/MK_Tree.h"
#include "../inference/inference.h"

using namespace std;

class VMS_Client_cls {
public:
    VMS_Client_cls(const string& ip, int port);
    ~VMS_Client_cls();

    void send_pubkey_to_server(const string& publicKey);
    //void send_image(queue<string>& CIDQueue);
    void send_image(Media_cls& media_inst, MK_Tree_cls& mk_tree_inst, Sign_cls& sign_inst, Inference& inference_inst);

    void start_send_image_thread(Media_cls& media_inst, MK_Tree_cls& mk_tree_inst, Sign_cls& sign_inst, Inference& inference_inst); // t7 스레드를 시작하는 함수
    void send_image_task(Media_cls& media_inst, MK_Tree_cls& mk_tree_inst, Sign_cls& sign_inst, Inference& inference_inst);         // t7 스레드의 실제 작업 함수
private:
    string server_ip;
    string server_port;

    thread send_image_thread;
    
    void init_libcurl();
    string create_metadata(const string& mk_root_hash, const string& sign_hash, const string& cid, const ODResult& result);

};

#endif // VMS_CLIENT_H
