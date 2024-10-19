#include "Media.h"

Media_cls::Media_cls(const Config_cls& config) {
    width = config.getWidth();
    height = config.getHeight();
    fps = config.getFps();
    frame_count = config.getFrameCount();
    orifile_path = config.getOrifilePath();
    pthread_mutex_init(&frameLocker, NULL);
    open_camera();
    lamping_time();
}

Media_cls::~Media_cls() {
    pthread_mutex_destroy(&frameLocker);
}

void Media_cls::open_camera() {
    int deviceID = 0;
    int apiID = CAP_V4L2;
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        cerr << "camera doesn't open" << endl;
        exit(0);
    }
}

void Media_cls::lamping_time() {
    Mat temp;
    if (!cap.isOpened())
    {
        cerr << "ERROR! Unable to open camera -- lamping time\n";
    }
    for (int i = 0; i < 20; i++) {
        cap >> temp;
    }
    temp.release();
}

int Media_cls::init_frame() {
    //cout << "----Initalizing---------" << endl;

    //camera_cfg_recv(width, height, fps);

    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);
    cap.set(CAP_PROP_FPS, fps);

    // cout << "    Frame Width: " << cvRound(cap.get(CAP_PROP_FRAME_WIDTH)) << endl;
    // cout << "    Frame Height: " << cvRound(cap.get(CAP_PROP_FRAME_HEIGHT)) << endl;
    // cout << "    FPS : " << cvRound(cap.get(CAP_PROP_FPS)) << endl;

    Mat img(Size(width, height), CV_8UC3, Scalar(0));
    this->frame = img.clone();
    img.release();

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    //cout << "----Initalized----------" << endl;
    return 0;
}

void* Media_cls::UpdateFrame(void* arg) {
    Media_cls* media = static_cast<Media_cls*>(arg);
    while (true) {
        Mat tempFrame(Size(media->width, media->height), CV_8UC3);
        media->cap >> tempFrame;

        if (tempFrame.empty()) {
            cout << "Failed to capture frame" << endl;
            continue;
        }

        pthread_mutex_lock(&media->frameLocker);
        media->frame = tempFrame.clone();
        pthread_mutex_unlock(&media->frameLocker);
    }
    pthread_exit((void*)0);
}

void Media_cls::capture_frame() {
    //cout << endl << "----Starting Capturing" << endl << endl;
    pthread_create(&UpdThread, NULL, Media_cls::UpdateFrame, this);

    int m_captured_frames = 0;

    while (true) {
        Mat currentFrame(Size(width, height), CV_8UC3);

        pthread_mutex_lock(&frameLocker);
        currentFrame = this->frame;
        pthread_mutex_unlock(&frameLocker);

        int sum1 = (int)sum(currentFrame)[0];
        int sum2 = (int)sum(currentFrame)[1];
        int sum3 = (int)sum(currentFrame)[2];
        int elementmean = (sum1 + sum2 + sum3) / 3;

        if (currentFrame.empty()) {
            cout << "Frame is empty" << endl;
        } else if (elementmean != 0) {
            bgr_cid_mtx.lock();
            bgr_queue.push(currentFrame);
            string s_cid = getCID();
            cid_queue_temp.push(s_cid);
            bgr_cid_mtx.unlock();

            m_captured_frames++;
        } else {
            //cout << "lamping time" << endl;
        }

        if (m_captured_frames == frame_count) {
            int ret = pthread_cancel(UpdThread);
            int status;
            if (ret == 0) {
                ret = pthread_join(UpdThread, (void **)&status);
                if (ret == 0) {
                    cout << "Capture End Successfully." << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                } else {
                    cout << "Thread End Error!" << ret << endl;
                    pthread_mutex_destroy(&frameLocker);
                    currentFrame.release();
                }
                break;
            }
        }
    }
}

void Media_cls::convert_frames2gray() {
    Mat original;
    string cid;
    Mat temp;

    bgr_cid_mtx.lock();
    bgr_queue.front().copyTo(original);
    bgr_queue.pop();
    cid = cid_queue_temp.front();
    cid_queue.push(cid);
    cid_queue_temp.pop();
    bgr_cid_mtx.unlock();

    string img_name = orifile_path + cid + ".jpg";
    imwrite(img_name, original);

    cvtColor(original, temp, COLOR_BGR2GRAY);

    g_queue_mtx.lock();
    G_queue.push(temp);
    g_queue_mtx.unlock();

    original.release();
    temp.release();
}

void Media_cls::edge_detection_BGR() {
    Mat temp;
    Mat edge_result;
        
    g_queue_mtx.lock();
    G_queue.front().copyTo(temp);
    G_queue.pop();
    g_queue_mtx.unlock();

    Canny(temp, edge_result, 20, 40);

    feature_vector_queue_mtx.lock();
    feature_vector_queue.push(edge_result);
    feature_vector_queue_mtx.unlock();

    temp.release();
    edge_result.release();
}

void Media_cls::capture_frame_task() {
    while (true) {
        if (init_frame() == -1) {
            cerr << "Error: init_frame failed." << endl;
            break;
        }
        capture_frame();
        cout << "    cid_queue size : " << cid_queue.size() << endl;
        this_thread::sleep_for(chrono::milliseconds(800));
    }
}

void Media_cls::start_capture_thread() {
    capture_thread = thread(&Media_cls::capture_frame_task, this);
}

void Media_cls::convert_frames2gray_task() {
    auto last_print_time = chrono::steady_clock::now();
    
    while (true) {
        if(!bgr_queue.empty()){
            convert_frames2gray();
        }
        // 현재 시간을 확인하여 마지막 출력 시간과의 차이를 계산
        auto current_time = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(current_time - last_print_time).count();

        // 1초마다 한 번씩만 출력
        if (elapsed >= 1) {
            cout << "    G_queue size : " << G_queue.size() << endl;
            last_print_time = current_time; // 마지막 출력 시간 갱신
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Media_cls::start_convert_frames2gray_thread() {
    convert_thread = thread(&Media_cls::convert_frames2gray_task, this);
}

void Media_cls::edge_detection_task() {
    auto last_print_time = chrono::steady_clock::now();
    while (true) {
        if(!G_queue.empty()){
            edge_detection_BGR();
        }
        auto current_time = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(current_time - last_print_time).count();
        if (elapsed >= 1) {
            cout << "    feature_vector_queue size : " << feature_vector_queue.size() << endl;
            last_print_time = current_time;
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

void Media_cls::start_edge_detection_thread() {
    edge_thread = thread(&Media_cls::edge_detection_task, this);
}
