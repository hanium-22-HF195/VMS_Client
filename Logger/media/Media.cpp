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
    cout << "----Initalizing---------" << endl;

    //camera_cfg_recv(width, height, fps);

    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);
    cap.set(CAP_PROP_FPS, fps);

    cout << "    Frame Width: " << cvRound(cap.get(CAP_PROP_FRAME_WIDTH)) << endl;
    cout << "    Frame Height: " << cvRound(cap.get(CAP_PROP_FRAME_HEIGHT)) << endl;
    cout << "    FPS : " << cvRound(cap.get(CAP_PROP_FPS)) << endl;

    Mat img(Size(width, height), CV_8UC3, Scalar(0));
    this->frame = img.clone();
    img.release();

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cout << "----Initalized----------" << endl;
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
    cout << endl << "----Starting Capturing" << endl << endl;
    pthread_create(&UpdThread, NULL, Media_cls::UpdateFrame, this);

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
            bgr_queue.push(currentFrame);
            string s_cid = getCID();
            cid_queue.push(s_cid);
        } else {
            cout << "lamping time" << endl;
        }

        if (bgr_queue.size() == frame_count) {
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
    cout << endl << "----Start to convert Frames into Grayscale----" << endl << endl;
    
    while (!bgr_queue.empty()) {
        Mat original = bgr_queue.front();
        bgr_queue.pop();

        //string img_name = orifile_path + getCID() + ".jpg"; // CID는 캡쳐 되는 동시에 생성되야해서 수정 필요
        
        string cid = cid_queue.front();
        cid_queue_temp.push(cid);
        string img_name = orifile_path + cid + ".jpg";

        cid_queue.pop();

        imwrite(img_name, original);
        Mat temp;

        cvtColor(original, temp, COLOR_BGR2GRAY);

        G_queue.push(temp);

        original.release();
        temp.release();
    }

    cout << "    Gray scale frame are saved" << endl;
    cout << "    Grayscale :  " << G_queue.size() << endl;
    cout << "----FRAMES CONVERTED---------" << endl << endl;
}

void Media_cls::edge_detection_BGR() {
    cout << "----Building feature vectors." << endl;

    while (!G_queue.empty()) {
        Mat temp;

        Canny(G_queue.front(), temp, 20, 40);

        feature_vector_queue.push(temp);
        G_queue.pop();
        temp.release();
    }
    cout << endl << "    Edge Detection made: " << feature_vector_queue.size() << endl;
}

void Media_cls::clearQueue() {
    while (!bgr_queue.empty()) bgr_queue.pop();
    while (!cid_queue.empty()) cid_queue.pop();
    while (!G_queue.empty()) G_queue.pop();
    while (!feature_vector_queue.empty()) feature_vector_queue.pop();
}
