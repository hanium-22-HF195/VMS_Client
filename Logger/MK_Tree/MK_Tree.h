#ifndef MK_TREE_H
#define MK_TREE_H

#include <queue>
#include <opencv2/opencv.hpp>
#include <string>
#include "../queue/msg_queue.h"
#include "Merkle_Tree/merkle_tree.h"

using namespace std;
using namespace cv;

class MK_Tree_cls {
public:
    MK_Tree_cls();
    void make_hash(queue<cv::Mat>& FV_QUEUE);
    void clearQueue();

    queue<string>& getHashQueue(){ return hash_queue; };
private:
    queue<string> hash_queue;
};

#endif
