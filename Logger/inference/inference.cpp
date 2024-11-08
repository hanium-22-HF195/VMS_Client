#include "inference.h"

Inference::Inference(const Config_cls& config) {
    m_server_ip_ = config.getObjectDetectorIp();
    m_server_port_ = config.getObjectDetectorPort();
    m_image_path_ = config.getOrifilePath();
}

size_t WriteCallback_OD(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Inference::send_request(queue<matadata>& matadata_queue) {
    string m_cid;
    
    ODResult result;
    result.label = "cat, remote, cat";
    result.prob = "1.0, 1.0, 1.0";
    result.positionbox = "{\"xmin\":14.24,\"ymin\":52.65,\"xmax\":309.68,\"ymax\":474.48},"
                         "{\"xmin\":40.58,\"ymin\":72.04,\"xmax\":175.05,\"ymax\":120.57},"
                         "{\"xmin\":343.42,\"ymin\":21.9,\"xmax\":639.54,\"ymax\":371.15}"; 
    result.objectcount = 3; 

    matadata_queue.front().object_Detection_result = result;

    this_thread::sleep_for(chrono::milliseconds(50));
}


void Inference::send_request_task(queue<matadata>& matadata_queue) {
    pthread_setname_np(pthread_self(), "thread 3");
    while (true) {
        if(!matadata_queue.empty()){
            if(!matadata_queue.front().cid.empty() && matadata_queue.front().object_Detection_result.label.empty()){
                auto total_start_time = chrono::steady_clock::now(); // 총 시간 시작
                send_request(matadata_queue);
                auto total_end_time = chrono::steady_clock::now(); // 총 시간 끝
                cout << "T3: "
                    << chrono::duration_cast<chrono::milliseconds>(total_end_time - total_start_time).count()
                    << " ms" << endl;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10)); 
    }
}

void Inference::start_send_request_thread(queue<matadata>& matadata_queue) {
    objectDetection_thread = thread(&Inference::send_request_task, this, ref(matadata_queue));
}