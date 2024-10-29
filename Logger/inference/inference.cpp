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

// void Inference::send_request(queue<string>& CID_queue) {
//     queue<string> temp_queue = CID_queue;

//     while (!temp_queue.empty()) {
//         string cid = temp_queue.front();
//         temp_queue.pop();

//         string imagePath = m_image_path_ + cid + ".jpg";

//         ifstream file(imagePath);
//         if (!file) {
//             cerr << "Image file not found: " << imagePath << endl;
//             exit(-1); // 파일이 없으면 종료
//         }

//         CURL* curl;
//         CURLcode res;
//         string response;

//         curl = curl_easy_init();

//         if(curl) {
//             ostringstream url;
//             url << "http://" << m_server_ip_ << ":" << m_server_port_ << "/detect"; 

//             curl_httppost* formpost = nullptr;
//             curl_httppost* lastptr = nullptr;

//             curl_formadd(&formpost, &lastptr,
//                          CURLFORM_COPYNAME, "image", 
//                          CURLFORM_FILE, imagePath.c_str(), 
//                          CURLFORM_END);

//             // 서버 URL 설정
//             curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());

//             curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            
//             //콜백함수 파일을 따로 생성하여 VMS_Client class와 inference class에 사용하도록 수정
//             curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_OD);  
//             curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

//             res = curl_easy_perform(curl);

//             if (res == CURLE_OK) {
//                 m_OD_result_queue_.push(response); // struct를 사용하여 데이터 전처리 기능 필요 
//                 cout << response << endl;
//             } else {
//                 cerr << "Error: " << curl_easy_strerror(res) << endl;
//             }

//             curl_formfree(formpost);  
//             curl_easy_cleanup(curl); 
//         }
//     }
// }
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