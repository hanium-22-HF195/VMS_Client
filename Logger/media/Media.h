#ifndef MEDIA_H
#define MEDIA_H

#include <sys/timeb.h>
#include <iostream>
#include "../config/Config.h"
#include "../queue/msg_queue.h"
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <pthread.h>
#include "create_cid.h"

using namespace std;
using namespace cv;

class Media_cls {
public:
    Media_cls(const Config_cls& config);
    ~Media_cls();

    void open_camera();
    void lamping_time();
    int init_frame();
    static void* UpdateFrame(void* arg);
    void capture_frame();
    void convert_frames2gray();
    void edge_detection_BGR();

    queue<cv::Mat>& getBGRQueue() { return bgr_queue; }
    queue<cv::Mat>& getGQueue() { return G_queue; }
    queue<cv::Mat>& getFeatureVectorQueue() { return feature_vector_queue; } 
    queue<string>& getCIDQueue() { return cid_queue_temp; } 
    void clearQueue();

private:
    int width, height, fps, frame_count;
    string orifile_path;
    pthread_mutex_t frameLocker;
    pthread_t UpdThread;
    VideoCapture cap;
    Mat frame;

    queue<Mat> bgr_queue;
    queue<string> cid_queue;
    queue<string> cid_queue_temp;
    queue<Mat> G_queue;
    queue<Mat> feature_vector_queue;
};

#endif
