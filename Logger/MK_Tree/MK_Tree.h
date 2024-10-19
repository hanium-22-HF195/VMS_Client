#ifndef MK_TREE_H
#define MK_TREE_H

#include <queue>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include "../media/Media.h"
#include "Merkle_Tree/merkle_tree.h"

using namespace std;
using namespace cv;

class MK_Tree_cls {
public:
    MK_Tree_cls();
    void make_hash(queue<cv::Mat>& FV_QUEUE, mutex& feature_vector_queue_mtx);

    queue<string>& getHashQueue(){ return hash_queue; };
    mutex& getHashQueueMutex() { return hash_queue_mtx; }

    void start_make_hash_thread(Media_cls& media_inst); // t5 스레드를 시작하는 함수
    void make_hash_task(Media_cls& media_inst);
    
private:
    queue<string> hash_queue;

    thread make_hash_thread; 
    mutex hash_queue_mtx;
};

#endif
