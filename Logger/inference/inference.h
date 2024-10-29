#ifndef INFERENCE_H
#define INFERENCE_H

#include "inference.h"
#include "../config/Config.h"
#include "../media/Media.h"
#include "../matadata/matadata.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring> 
#include <queue>
#include <thread>
#include <pthread.h>
#include <mutex>
#include <jsoncpp/json/json.h>

using namespace std;

class Inference {
public:
    Inference(const Config_cls& config);

    void send_request(queue<matadata>& matadata_queue);
    queue<ODResult>& getODResultQueue() { return OD_result_queue; }

    void start_send_request_thread(queue<matadata>& matadata_queue);
    void send_request_task(queue<matadata>& matadata_queue);
    mutex& getODResultQueueMutex() { return OD_result_queue_mtx; }

private:
    thread objectDetection_thread;
    string m_server_ip_;
    int m_server_port_;
    string m_image_path_;
    queue<ODResult> OD_result_queue;
    mutex OD_result_queue_mtx;
    
};




#endif // INFERENCE_H
