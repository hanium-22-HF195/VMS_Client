#ifndef MEDIA_H
#define MEDIA_H

#include <sys/timeb.h>
#include <iostream>
#include <fstream>
#include "../config/Config.h"
#include "../matadata/matadata.h"
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <mutex>
#include <pthread.h>
#include <thread>
#include "create_cid.h"

#include <spdlog/spdlog.h>

using namespace std;
using namespace cv;

class Media_cls {
public:
    Media_cls(const Config_cls& config);
    ~Media_cls();

    void open_camera();
    void lamping_time();
    int set_frame();
    static void* UpdateFrame(void* arg);
    void capture_frame(queue<matadata>& matadata_queue, mutex& matadata_mutex);
    void image_save(queue<matadata>& matadata_queue);
    void convert_frames2gray(queue<matadata>& matadata_queue);
    void edge_detection_BGR(queue<matadata>& matadata_queue);

    // queue<cv::Mat>& getBGRQueue() { return bgr_queue; }
    // queue<cv::Mat>& getGQueue() { return G_queue; }
    // queue<cv::Mat>& getFeatureVectorQueue() { return feature_vector_queue; } 
    // queue<string>& getCIDQueue() { return cid_queue; } 
    Mat getEdgeResult() { return m_edge_result; }

    void start_capture_thread(queue<matadata>& matadata_queue, mutex& matadata_mutex);
    void start_capture_save_thread(queue<matadata>& matadata_queue);
    void start_convert_frames2gray_thread(queue<matadata>& matadata_queue);
    void start_edge_detection_thread(queue<matadata>& matadata_queue);           

    void capture_frame_task(queue<matadata>& matadata_queue, mutex& matadata_mutexs);
    void capture_image_save_task(queue<matadata>& matadata_queue);
    void convert_frames2gray_task(queue<matadata>& matadata_queue);                                      
    void edge_detection_task(queue<matadata>& matadata_queue);                                            

    // mutex& getFeatureVectorQueueMutex() { return feature_vector_queue_mtx; }
    // mutex& getCIDQueueMutex() { return bgr_cid_mtx; }

private:
    int width, height, fps, frame_count;
    string orifile_path;
    // pthread_mutex_t frameLocker;
    // pthread_t UpdThread;
    VideoCapture cap;
    //Mat frame;
    Mat currentFrame;
    Mat m_G_frame;
    Mat m_edge_result;

    // queue<Mat> bgr_queue;
    // queue<string> cid_queue;
    // queue<string> cid_queue_temp;
    // queue<Mat> G_queue;
    // queue<Mat> feature_vector_queue;

    //스레드
    thread capture_thread;
    thread image_save_thread;
    thread convert_thread;
    thread edge_thread;
};

#endif
