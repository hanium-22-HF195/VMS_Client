#include "VMS_Client.h"
#include <iostream>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

VMS_Client_cls::VMS_Client_cls(const string& ip, int port) : server_ip(ip), server_port(to_string(port)) {
    init_libcurl();  
}

VMS_Client_cls::~VMS_Client_cls() {
    curl_global_cleanup();  
}

void VMS_Client_cls::init_libcurl() {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if(res != CURLE_OK) {
        cerr << "curl_global_init() failed: " << curl_easy_strerror(res) << endl;
        exit(-1); 
    }
}

void VMS_Client_cls::send_pubkey_to_server(const string& publicKey) {
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

string VMS_Client_cls::create_metadata(const string& mk_root_hash, 
                                        const string& sign_hash, 
                                        const string& cid, 
                                        const ODResult& result) {
    Json::Value metadata;
    metadata["MK_root_hash"] = mk_root_hash;
    metadata["sign_hash"] = sign_hash;
    metadata["CID"] = cid;
    metadata["mediaType"] = "BGR";

    metadata["label"] = result.label;
    metadata["prob"] = result.prob;
    metadata["positionbox"] = result.positionbox;
    metadata["objectcount"] = result.objectcount;

    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, metadata);
}

void VMS_Client_cls::send_image(Media_cls& media_inst, 
                                MK_Tree_cls& mk_tree_inst, 
                                Sign_cls& sign_inst, 
                                Inference& inference_inst) {
    string cid;
    media_inst.getCIDQueueMutex().lock();
    cid = media_inst.getCIDQueue().front();
    media_inst.getCIDQueue().pop();
    media_inst.getCIDQueueMutex().unlock();

    string mk_root_hash;
    string sign_hash;
    sign_inst.getSignHashQueueMutex().lock();
    mk_root_hash = sign_inst.getHashPairQueue().front().hash;
    sign_hash = sign_inst.getHashPairQueue().front().sign_hash;
    sign_inst.getHashPairQueue().pop();
    sign_inst.getSignHashQueueMutex().unlock();

    ODResult od_result;
    inference_inst.getODResultQueueMutex().lock();
    od_result = inference_inst.getODResultQueue().front();
    inference_inst.getODResultQueue().pop();
    inference_inst.getODResultQueueMutex().unlock();

    string imagePath = "/home/pi/images/" + cid + ".jpg";

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Error initializing libcurl." << endl;
        return;
    }

    string uploadUrl = "http://" + server_ip + ":" + server_port + "/test/data";
    curl_easy_setopt(curl, CURLOPT_URL, uploadUrl.c_str());

    string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    curl_httppost* formpost = nullptr;
    curl_httppost* lastptr = nullptr;

    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "imagedata",
                 CURLFORM_FILE, imagePath.c_str(),
                 CURLFORM_END);

    string metadata = create_metadata(mk_root_hash, sign_hash, cid, od_result);
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "metadata",
                 CURLFORM_COPYCONTENTS, metadata.c_str(),
                 CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "Error during HTTP request: " << curl_easy_strerror(res) << endl;
    } else {
        //cout << "Response Message: " << responseData << endl;
    }
    curl_formfree(formpost);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        cerr << "Error during upload: " << curl_easy_strerror(res) << endl;
    }
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void VMS_Client_cls::send_image_task(Media_cls& media_inst, 
                                        MK_Tree_cls& mk_tree_inst, 
                                        Sign_cls& sign_inst, 
                                        Inference& inference_inst) {
    while (true) {
        if (!media_inst.getCIDQueue().empty() &&
             !sign_inst.getHashPairQueue().empty() && 
             !inference_inst.getODResultQueue().empty()) {
            send_image(media_inst, 
                        mk_tree_inst, 
                        sign_inst, 
                        inference_inst);
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void VMS_Client_cls::start_send_image_thread(Media_cls& media_inst, 
                                                MK_Tree_cls& mk_tree_inst, 
                                                Sign_cls& sign_inst, 
                                                Inference& inference_inst) {
                                                    
    send_image_thread = thread(&VMS_Client_cls::send_image_task, this, 
                                ref(media_inst), 
                                ref(mk_tree_inst), 
                                ref(sign_inst), 
                                ref(inference_inst)); 
}

// void VMS_Client_cls::send_image_task(Media_cls& media_inst, 
//                                      MK_Tree_cls& mk_tree_inst, 
//                                      Sign_cls& sign_inst, 
//                                      Inference& inference_inst) {
//     while (true) {
//         // 뮤텍스 범위 내에서 두 큐의 상태를 동시에 확인
//         bool canSendImage = false;

//         {
//             lock_guard<mutex> lock1(sign_inst.getSignHashQueueMutex());   // SignHashQueue 뮤텍스 잠금
//             lock_guard<mutex> lock2(inference_inst.getODResultQueueMutex()); // ODResultQueue 뮤텍스 잠금

//             if (!media_inst.getCIDQueue().empty() &&
//                  !mk_tree_inst.getHashQueue().empty() && 
//                  !sign_inst.getSignHashQueue().empty() && 
//                  !inference_inst.getODResultQueue().empty()) {
//                 canSendImage = true;
//             }
//         }

//         if (canSendImage) {
//             // send_image 함수가 실행되기 전 시간 기록
//             auto start = std::chrono::high_resolution_clock::now();

//             // send_image 호출
//             send_image(media_inst, mk_tree_inst, sign_inst, inference_inst);

//             // send_image 함수가 끝난 후 시간 기록
//             auto end = std::chrono::high_resolution_clock::now();

//             // 걸린 시간 계산 (밀리초 단위)
//             std::chrono::duration<double, std::milli> elapsed = end - start;
//             cout << "send_image 실행 시간: " << elapsed.count() << " ms" << endl;
//         }

//         this_thread::sleep_for(chrono::milliseconds(10)); // 10ms 대기
//     }
// }



