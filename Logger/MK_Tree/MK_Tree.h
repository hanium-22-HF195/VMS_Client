#ifndef MK_TREE_H
#define MK_TREE_H

#include <queue>
#include <opencv2/opencv.hpp>
#include <string>
#include "../queue/msg_queue.h"
#include "Merkle_Tree/merkle_tree.h"

using namespace std;
using namespace cv;

class MK_Tree {
public:
    MK_Tree();
    void make_hash(queue<cv::Mat>& FV_QUEUE);
    queue<string>& getHashQueue();

private:
    queue<string> hash_queue;
};

#endif
