#include <iostream>
#include "config/Config.h"
#include "sign/sign.h"
#include "VMS_Client/VMS_Client.h"
#include "media/Media.h"
#include "MK_Tree/MK_Tree.h"
#include "inference/inference.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unistd.h> 

using namespace std;

void process_loop() {
    while (true) {
        usleep(1000000); //1초
    }
}

int main() {
    Config_cls config_inst;

    Sign_cls sign_inst(config_inst.getSignedHashBufSize());
    VMS_Client_cls client_inst(config_inst.getServerIp(), config_inst.getServerPort());
    client_inst.send_pubkey_to_server(sign_inst.getPublicKey());
    Inference inference_inst(config_inst);
    Media_cls media_inst(config_inst);
    MK_Tree_cls mk_tree_inst;

    media_inst.start_capture_thread();
    media_inst.start_convert_frames2gray_thread();
    inference_inst.start_send_request_thread(media_inst); 
    media_inst.start_edge_detection_thread();
    mk_tree_inst.start_make_hash_thread(media_inst);
    sign_inst.start_sign_hash_thread(mk_tree_inst);
    client_inst.start_send_image_thread(media_inst, mk_tree_inst, sign_inst, inference_inst);

    process_loop();

    return 0;
}
