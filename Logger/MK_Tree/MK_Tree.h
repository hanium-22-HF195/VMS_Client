#ifndef MK_TREE_H
#define MK_TREE_H

#include <fstream>
#include <chrono>
#include <iostream>
#include <queue>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include "../media/Media.h"
#include "Merkle_Tree/merkle_tree.h"

#include <spdlog/spdlog.h>

using namespace std;
using namespace cv;

class MK_Tree_cls {
public:
    MK_Tree_cls();
    void make_hash(queue<matadata>& matadata_queue, Media_cls& media_inst);

    queue<string>& getHashQueue(){ return hash_queue; };
    mutex& getHashQueueMutex() { return hash_queue_mtx; }

    void start_make_hash_thread(queue<matadata>& matadata_queue, Media_cls& media_inst); // t5 스레드를 시작하는 함수
    void make_hash_task(queue<matadata>& matadata_queue, Media_cls& media_inst);
    
private:
    queue<string> hash_queue;

    thread make_hash_thread; 
    mutex hash_queue_mtx;
};

#endif
