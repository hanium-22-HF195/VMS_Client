#ifndef MATADATA_H
#define MATADATA_H

#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

struct ODResult {
    string label;
    string prob;         
    string positionbox; 
    int objectcount;     
};

struct matadata {
    string cid;
    Mat BGR_frame;
    bool G_frame_state = false;
    bool feature_vector_state = false;
    string hash;
    string sign_hash;
    ODResult object_Detection_result;
};

#endif // MATADATA_H
