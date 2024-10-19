#ifndef MEDIA_H
#define MEDIA_H

#include <sys/timeb.h>
#include <iostream>
#include "../config/Config.h"
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <mutex>
#include <pthread.h>
#include <thread>
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
    queue<string>& getCIDQueue() { return cid_queue; } 

    void start_capture_thread();              // t1 스레드를 시작하는 함수
    void start_convert_frames2gray_thread();  // t2 스레드를 시작하는 함수
    void start_edge_detection_thread();       // t4 스레드를 시작하는 함수

    void capture_frame_task();
    void convert_frames2gray_task();          // t2 스레드의 실제 작업 함수
    void edge_detection_task();               // t4 스레드의 실제 작업 함수

    mutex& getFeatureVectorQueueMutex() { return feature_vector_queue_mtx; }
    mutex& getCIDQueueMutex() { return bgr_cid_mtx; }

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

    //스레드
    thread capture_thread;
    thread convert_thread;
    thread edge_thread;

    // 뮤텍스들
    mutex bgr_cid_mtx;  // bgr_queue와 cid_queue를 보호
    mutex g_queue_mtx;   // G_queue를 보호
    mutex feature_vector_queue_mtx;   // feature_vector_queue를 보호
};

#endif
