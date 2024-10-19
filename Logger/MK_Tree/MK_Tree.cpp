#include "MK_Tree.h"

MK_Tree_cls::MK_Tree_cls() {

}

void MK_Tree_cls::make_hash(queue<cv::Mat>& FV_QUEUE, mutex& feature_vector_queue_mtx)
{
    cv::Mat m_temp;

    feature_vector_queue_mtx.lock();
    FV_QUEUE.front().copyTo(m_temp);
    FV_QUEUE.pop();
    feature_vector_queue_mtx.unlock();

    //to_string을 사용하여 string 형변환 하게되면 성능이 안좋을 수 있어 byte형으로 바꾸어야 함.
    string mat_data = "";
    string sha_result = "";

    for (int i = 0; i < m_temp.rows; i++)
    {
        for (int j = 0; j < m_temp.cols; j++)
        {
            mat_data += to_string(m_temp.at<uchar>(i, j)); 
        }
    }

    sha_result = hash_sha256(mat_data);
    //cout << "sha_result : " << sha_result << endl;

    hash_queue_mtx.lock();
    hash_queue.push(sha_result);
    hash_queue_mtx.unlock();

    m_temp.release();
}

void MK_Tree_cls::make_hash_task(Media_cls& media_inst) {
    auto last_print_time = chrono::steady_clock::now();
    while (true) {
        if(!media_inst.getFeatureVectorQueue().empty()){
            make_hash(media_inst.getFeatureVectorQueue(), media_inst.getFeatureVectorQueueMutex());
        }
        auto current_time = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(current_time - last_print_time).count();
        if (elapsed >= 1) {
            cout << "    hash_queue size : " << hash_queue.size() << endl;
            last_print_time = current_time;
        }
        this_thread::sleep_for(chrono::milliseconds(20));
    }
}


void MK_Tree_cls::start_make_hash_thread(Media_cls& media_inst) {
    make_hash_thread = thread(&MK_Tree_cls::make_hash_task, this, ref(media_inst));
}