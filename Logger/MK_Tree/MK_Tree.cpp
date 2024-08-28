#include "MK_Tree.h"

MK_Tree_cls::MK_Tree_cls() {

}

void MK_Tree_cls::make_hash(queue<cv::Mat>& FV_QUEUE)
{
    queue<cv::Mat> Feature_Vector_queue(FV_QUEUE);
    cout << endl
         << "----Make HASH from feature vectors." << endl
         << endl;

    int mat_width = Feature_Vector_queue.front().cols;
    int mat_height = Feature_Vector_queue.front().rows;
    int mat_channels = Feature_Vector_queue.front().channels();
    int umat_data_bufsize = mat_width * mat_height * mat_channels;

    while (!Feature_Vector_queue.empty())
    {
        cv::Mat temp = Feature_Vector_queue.front();
        Feature_Vector_queue.pop();

        string mat_data = "";
        string sha_result = "";

        for (int i = 0; i < temp.rows; i++)
        {
            for (int j = 0; j < temp.cols; j++)
            {
                mat_data += to_string(temp.at<uchar>(i, j));
            }
        }

        sha_result = hash_sha256(mat_data);
        cout << "sha_result : " << sha_result << endl;

        hash_queue.push(sha_result);
        temp.release();
    }
    cout << "    hash made : " << hash_queue.size() << endl;
}

void MK_Tree_cls::clearQueue() {
    while (!hash_queue.empty()) hash_queue.pop();
}