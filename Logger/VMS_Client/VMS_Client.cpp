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

string VMS_Client_cls::create_metadata(const string& mk_root_hash, const string& sign_hash, const string& cid) {
    Json::Value metadata;
    metadata["MK_root_hash"] = mk_root_hash;
    metadata["sign_hash"] = sign_hash;
    metadata["CID"] = cid;
    metadata["mediaType"] = "BGR";

    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, metadata);
}

void VMS_Client_cls::send_image(queue<string>& CIDQueue, queue<string>& MKRootQueue, queue<string>& SignHashQueue) {
    while (!CIDQueue.empty() && !MKRootQueue.empty() && !SignHashQueue.empty()) {
        string cid = CIDQueue.front();
        CIDQueue.pop();

        string mk_root_hash = MKRootQueue.front();
        MKRootQueue.pop();

        string sign_hash = SignHashQueue.front();
        SignHashQueue.pop();

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

        string metadata = create_metadata(mk_root_hash, sign_hash, cid);
        curl_formadd(&formpost, &lastptr,
                     CURLFORM_COPYNAME, "metadata",
                     CURLFORM_COPYCONTENTS, metadata.c_str(),
                     CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            cerr << "Error during HTTP request: " << curl_easy_strerror(res) << endl;
        } else {
            cout << "Response Message: " << responseData << endl;
        }

        curl_formfree(formpost);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            cerr << "Error during upload: " << curl_easy_strerror(res) << endl;
        }
    }
}


size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}
