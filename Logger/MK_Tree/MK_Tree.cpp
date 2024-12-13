#include "MK_Tree.h"

MK_Tree_cls::MK_Tree_cls() {

}

void MK_Tree_cls::make_hash(queue<matadata>& matadata_queue, Media_cls& media_inst)
{
    cv::Mat m_temp;
    media_inst.getEdgeResult().copyTo(m_temp);

    string mat_data = "";
    string sha_result = "";
    for (int i = 0; i < m_temp.rows; i++)
    {
        for (int j = 0; j < m_temp.cols; j++)
        {
            //to_string을 사용하여 string 형변환 하게되면 성능이 안좋을 수 있어 byte형으로 바꾸어야 함.
            mat_data += to_string(m_temp.at<uchar>(i, j));
        }
    }
    //auto total_start_time = std::chrono::steady_clock::now();
    sha_result = hash_sha256(mat_data);
    //auto total_end_time = std::chrono::steady_clock::now();
    //int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
    //std::cout << "sha_result : " << duration << " ms" << std::endl;
    matadata_queue.front().hash = sha_result;
    m_temp.release();
    
}

// 750ms
// void MK_Tree_cls::make_hash(queue<matadata>& matadata_queue, Media_cls& media_inst)
// {
//     cv::Mat m_temp;
//     media_inst.getEdgeResult().copyTo(m_temp);

//     string mat_data = "";
//     string sha_result = "";

//     cv::Mat oneRow = m_temp.reshape(0,1);
//     std::ostringstream os;
//     os << oneRow;                             // Put to the stream
//     std::string s_str = os.str();             // Get string 
//     s_str.pop_back();                         // Remove brackets
//     s_str.erase(0,1);


//     sha_result = hash_sha256(s_str);
//     matadata_queue.front().hash = sha_result;
//     m_temp.release();
// }

// void MK_Tree_cls::make_hash(queue<matadata>& matadata_queue, Media_cls& media_inst)
// {
//     cv::Mat m_temp;
//     media_inst.getEdgeResult().copyTo(m_temp);

//     std::vector<uchar> mat_data_vec(m_temp.begin<uchar>(), m_temp.end<uchar>());

//     std::ostringstream mat_data_stream;
//     for (auto val : mat_data_vec) {
//         mat_data_stream << static_cast<int>(val);
//     }
//     string mat_data = mat_data_stream.str();
//     string sha_result = hash_sha256(mat_data);
//     matadata_queue.front().hash = sha_result;

//     m_temp.release();
// }

// void MK_Tree_cls::make_hash_task(queue<matadata>& matadata_queue, Media_cls& media_inst) {
//     pthread_setname_np(pthread_self(), "thread 5");
//     while (true) {
//         if(!matadata_queue.empty()){
//             if(matadata_queue.front().feature_vector_state == true && matadata_queue.front().hash.empty()){
//                 auto total_start_time = chrono::steady_clock::now(); // 총 시간 시작
//                 make_hash(matadata_queue, media_inst);
//                 auto total_end_time = chrono::steady_clock::now(); // 총 시간 끝
//                 cout << "T5: "
//                     << chrono::duration_cast<chrono::milliseconds>(total_end_time - total_start_time).count()
//                     << " ms" << endl;
//             }
//         }
//         this_thread::sleep_for(chrono::milliseconds(20));
//     }
// }

void MK_Tree_cls::make_hash_task(queue<matadata>& matadata_queue, Media_cls& media_inst) {
    pthread_setname_np(pthread_self(), "thread 5");

    while (true) {
        if (!matadata_queue.empty()) {
            if (matadata_queue.front().feature_vector_state == true && matadata_queue.front().hash.empty()) {
                auto total_start_time = std::chrono::steady_clock::now();
                make_hash(matadata_queue, media_inst);
                auto total_end_time = std::chrono::steady_clock::now();

                int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
                spdlog::info("T5: {} ms", duration);

                std::cout << "T5: " << duration << " ms" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void MK_Tree_cls::start_make_hash_thread(queue<matadata>& matadata_queue, Media_cls& media_inst) {
    make_hash_thread = thread(&MK_Tree_cls::make_hash_task, this, ref(matadata_queue), ref(media_inst));
}

