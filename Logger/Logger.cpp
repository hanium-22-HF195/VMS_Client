#include <iostream>
#include "config/Config.h"
#include "sign/sign.h"
#include "VMS_Client/VMS_Client.h"
#include "media/Media.h"
#include "MK_Tree/MK_Tree.h"
#include "inference/inference.h"
#include "matadata/matadata.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unistd.h> 

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

void init_logging() {
    spdlog::init_thread_pool(8192, 8); // 큐 크기와 스레드 수 설정
    auto async_file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_logger", "thread_time_test.log");
    spdlog::set_default_logger(async_file_logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] %v");
    spdlog::info("Logger initialized.");
}

using namespace std;    

void process_loop() {
    while (true) {
        usleep(1000000); //1초
    }
}

int main() {
    init_logging();

    Config_cls config_inst;

    Sign_cls sign_inst(config_inst.getSignedHashBufSize());
    VMS_Client_cls client_inst(config_inst.getServerIp(), config_inst.getServerPort());
    client_inst.send_pubkey_to_server(sign_inst.getPublicKey());
    Inference inference_inst(config_inst);
    Media_cls media_inst(config_inst);
    MK_Tree_cls mk_tree_inst;

    //create matadata queue
    queue<matadata> matadata_queue;
    mutex matadata_mutex;

    media_inst.start_capture_thread(matadata_queue, matadata_mutex);
    media_inst.start_capture_save_thread(matadata_queue);
    media_inst.start_convert_frames2gray_thread(matadata_queue);
    inference_inst.start_send_request_thread(matadata_queue);
    media_inst.start_edge_detection_thread(matadata_queue);
    mk_tree_inst.start_make_hash_thread(matadata_queue, media_inst);
    sign_inst.start_sign_hash_thread(matadata_queue);
    client_inst.start_send_image_thread(matadata_queue, matadata_mutex);

    process_loop();

    return 0;
}
