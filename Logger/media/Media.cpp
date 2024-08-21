// media/Media.cpp
#include "Media.h"

Media::Media(const Config& config) {
    width = config.getWidth();
    height = config.getHeight();
    fps = config.getFps();
    frame_count = config.getFrameCount();
    orifile_path = config.getOrifilePath();
    pthread_mutex_init(&frameLocker, NULL);
    open_camera();
    lamping_time();
}

Media::~Media() {
    pthread_mutex_destroy(&frameLocker);
}

void Media::open_camera() {
    int deviceID = 0;
    int apiID = CAP_V4L2;
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        cerr << "camera doesn't open" << endl;
        exit(0);
    }
}

void Media::lamping_time() {
    Mat temp;
    for (int i = 0; i < 20; i++) {
        cap >> temp;
    }
    temp.release();
}

int Media::init_camera() {
    cout << "----Initalizing---------" << endl;

    camera_cfg_recv(width, height, fps);

    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);
    cap.set(CAP_PROP_FPS, fps);

    cout << "    Frame Width: " << cvRound(cap.get(CAP_PROP_FRAME_WIDTH)) << endl;
    cout << "    Frame Height: " << cvRound(cap.get(CAP_PROP_FRAME_HEIGHT)) << endl;
    cout << "    FPS : " << cvRound(cap.get(CAP_PROP_FPS)) << endl;

    Mat img(Size(width, height), CV_8UC3, Scalar(0));
    frame = img.clone();
    img.release();

    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cout << "----Initalized----------" << endl;
    return 0;
}

void* Media::UpdateFrame(void* arg) {
    Media* media = static_cast<Media*>(arg);
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

void Media::capture_frame() {
    cout << endl << "----Starting Capturing" << endl << endl;
    pthread_create(&UpdThread, NULL, Media::UpdateFrame, this);

    while (true) {
        Mat currentFrame(Size(width, height), CV_8UC3);

        pthread_mutex_lock(&frameLocker);
        currentFrame = frame;
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

void Media::convert_frames2gray() {
    cout << endl << "----Start to convert Frames into Grayscale----" << endl << endl;
    
    while (!bgr_queue.empty()) {
        Mat original = bgr_queue.front();
        bgr_queue.pop();

        string img_name = orifile_path + getCID() + ".jpg";
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

void Media::edge_detection_BGR() {
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

string Media::getCID() {
    struct timeb tb;
    struct tm tstruct;
    ostringstream oss;
    string s_CID;
    char buf[128];

    ftime(&tb);
    if (nullptr != localtime_r(&tb.time, &tstruct)) {
        strftime(buf, sizeof(buf), "%Y-%m-%d_%T.", &tstruct);
        oss << buf;
        oss << tb.millitm;
    }

    s_CID = oss.str();
    s_CID = s_CID.substr(0, 23);
    if (s_CID.length() == 22) s_CID.append("0");
    if (s_CID.length() == 21) s_CID.append("00");

    return s_CID;
}
