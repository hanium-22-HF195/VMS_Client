#include "VMS_Client.h"
#include <iostream>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

VMS_Client_cls::VMS_Client_cls(const string& ip, int port) : server_ip(ip), server_port(to_string(port)) {
    init_libcurl();  
    image_uploadUrl = "http://" + server_ip + ":" + server_port + "/test/data";
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

                                        
string VMS_Client_cls::create_metadata(const string& cid, 
                                        const string& hash, 
                                        const string& sign_hash, 
                                        const ODResult& ODresult) {
    Json::Value metadata;
    metadata["CID"] = cid;
    metadata["MK_root_hash"] = hash;
    metadata["sign_hash"] = sign_hash;
    metadata["mediaType"] = "BGR";

    metadata["label"] = ODresult.label;
    metadata["prob"] = ODresult.prob;
    metadata["positionbox"] = ODresult.positionbox;
    metadata["objectcount"] = ODresult.objectcount;

    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, metadata);
}

void VMS_Client_cls::send_image(queue<matadata>& matadata_queue, mutex& matadata_mutex) {

    if (matadata_queue.empty()) {
        cerr << "T7 Error: matadata_queue is empty" << endl;
        return;
    }

    string m_cid;
    m_cid = matadata_queue.front().cid;

    string m_hash;
    m_hash = matadata_queue.front().hash;

    string m_sign_hash;
    m_sign_hash = matadata_queue.front().sign_hash;

    ODResult m_OD_result;
    m_OD_result = matadata_queue.front().object_Detection_result;

    matadata_mutex.lock();
    matadata_queue.pop();
    matadata_mutex.unlock();

    string home_dir = getenv("HOME");
    string imagePath = home_dir +"/images/" + m_cid + ".jpg";
    //string imagePath = "/home/pi/images/" + m_cid + ".jpg";

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Error initializing libcurl." << endl;
        return;
    }

    //string image_uploadUrl = "http://" + server_ip + ":" + server_port + "/test/data";
    //cout << "image_uploadUrl : " << image_uploadUrl << endl;
    curl_easy_setopt(curl, CURLOPT_URL, image_uploadUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

    curl_httppost* formpost = nullptr;
    curl_httppost* lastptr = nullptr;

    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "imagedata",
                 CURLFORM_FILE, imagePath.c_str(),
                 CURLFORM_END);

    metadata = create_metadata(m_cid, m_hash, m_sign_hash, m_OD_result);
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
        cout << "Send data successfully"  << endl;
        remove(imagePath.c_str());
    }
    curl_formfree(formpost);
    curl_easy_cleanup(curl);

}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void VMS_Client_cls::send_image_task(queue<matadata>& matadata_queue, mutex& matadata_mutex) {
    pthread_setname_np(pthread_self(), "thread 7");
    while (true) {
        if(!matadata_queue.empty()){
            if (!matadata_queue.front().sign_hash.empty() && 
                !matadata_queue.front().object_Detection_result.label.empty() &&
                matadata_queue.front().image_save_state == true) {
                send_image(matadata_queue, matadata_mutex);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void VMS_Client_cls::start_send_image_thread(queue<matadata>& matadata_queue, mutex& matadata_mutex) {
                                                    
    send_image_thread = thread(&VMS_Client_cls::send_image_task, this, ref(matadata_queue), ref(matadata_mutex)); 
}
