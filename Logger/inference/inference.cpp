#include "inference.h"

#include <ctime>
#include <iostream>

Inference::Inference(const Config_cls& config) {
    m_server_ip_ = config.getObjectDetectorIp();
    m_server_port_ = config.getObjectDetectorPort();
    m_image_path_ = config.getOrifilePath();
}

size_t WriteCallback_OD_bak(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t WriteCallback_OD(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t numBytes = size *nmemb;

    static_cast<string*> (userp)->append(static_cast<char*>(contents), 0, numBytes);
    return numBytes;
}



bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Inference::send_request(queue<matadata>& matadata_queue) {
    string m_cid = matadata_queue.front().cid;
    string home_dir = getenv("HOME");
    string imagePath = home_dir + "/images/" + m_cid + ".png";

    if (is_file_exist(imagePath.c_str()) != true)
    {
        cerr << "(Inference)There is no file for sending to inference server " << endl;
        exit (-1);
    }
     
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Error initializing libcurl." << endl;
        return;
    }
    char buf_port[10];
    memset(buf_port, 0x00, 10);

    sprintf(buf_port, "%d", m_server_port_);
    string server_url = "http://" + m_server_ip_ + ":" + buf_port +"/detect_obj"; // Object Detection Rest Server에 맞게 경로 수정
    //cout << "object detection : " << server_url << endl;
    

    curl_httppost* formpost = nullptr;
    curl_httppost* lastptr = nullptr;

    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, imagePath.c_str(),
                 CURLFORM_CONTENTTYPE, "image/jpeg",
                 CURLFORM_END);
    
    //cout << "file path : " << imagePath.c_str() << endl;
    curl_slist * headerlist = curl_slist_append(NULL, "Expected:");
 
    
    
    curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_OD);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "(Object Detect)Error during HTTP request: " << curl_easy_strerror(res) << endl;
        

    } else {
        cout << "Response received: " << response << endl;

        std::time_t result = std::time(nullptr);
        std::cout << std::asctime(std::localtime(&result))
              << result << " seconds since the Epoch\n";

        matadata_queue.front().object_Detection_result = response;
        matadata_queue.front().object_Detection_result_state = true;
    }

    curl_formfree(formpost);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headerlist);
    curl_global_cleanup();
}


void Inference::send_request_task(queue<matadata>& matadata_queue) {
    pthread_setname_np(pthread_self(), "thread 3");
    while (true) {
        if(!matadata_queue.empty()){
            if(matadata_queue.front().image_save_state == true && 
                matadata_queue.front().object_Detection_result_state == false){
                auto total_start_time = chrono::steady_clock::now(); // 총 시간 시작
                send_request(matadata_queue);
                auto total_end_time = chrono::steady_clock::now(); // 총 시간 끝
                int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
                spdlog::info("T3: {} ms", duration);

                std::cout << "T3: " << duration << " ms" << std::endl;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10)); 
    }
}

void Inference::start_send_request_thread(queue<matadata>& matadata_queue) {
    objectDetection_thread = thread(&Inference::send_request_task, this, ref(matadata_queue));
}