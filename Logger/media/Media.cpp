#include "Media.h"

Media_cls::Media_cls(const Config_cls& config) {
    width = config.getWidth();
    height = config.getHeight();
    fps = config.getFps();
    frame_count = config.getFrameCount();
    orifile_path = config.getOrifilePath();
    open_camera();
    lamping_time();

}

Media_cls::~Media_cls() {
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

int Media_cls::set_frame() {
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, width);
    cap.set(CAP_PROP_FRAME_HEIGHT, height);
    cap.set(CAP_PROP_FPS, fps);

    cout << "    Frame Width: " << cvRound(cap.get(CAP_PROP_FRAME_WIDTH)) << endl;
    cout << "    Frame Height: " << cvRound(cap.get(CAP_PROP_FRAME_HEIGHT)) << endl;
    cout << "    FPS : " << cvRound(cap.get(CAP_PROP_FPS)) << endl;

    Mat img(Size(width, height), CV_8UC3, Scalar(0));
    this->currentFrame = img.clone();
    img.release();


    return 0;
}

//int capture_count = 0; //Global variables for testing
void Media_cls::capture_frame(queue<matadata>& matadata_queue, mutex& matadata_mutex) {
    unsigned int captured_threshold = 100;

    matadata data;
    while (true) {
        // if(capture_count >= 1000){
        //     spdlog::info("finish time test");
        //     break;
        // }
        if (matadata_queue.size() > captured_threshold) {
            cout << "Queue exceeds its threshold" << endl;
            spdlog::info("Queue exceeds its threshold");
            this_thread::sleep_for(chrono::milliseconds(800));
            lamping_time();
            continue;
        }
        auto frame_start_time = std::chrono::steady_clock::now();

        cap.read(currentFrame);
        auto frame_end_time = std::chrono::steady_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end_time - frame_start_time).count();

        data.cid = getCID();
        data.BGR_frame = currentFrame;
        matadata_queue.push(data);
        

        std::cout << "T0: " << duration << " ms" << std::endl;
        spdlog::info("T0: {} ms", duration);
        //capture_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        // this_thread::sleep_for(chrono::milliseconds(8000000));
    }
    data.BGR_frame.release();
}

void Media_cls::image_save(queue<matadata>& matadata_queue) {
    Mat original;
    string m_cid;

    m_cid = matadata_queue.front().cid;
    matadata_queue.front().BGR_frame.copyTo(original);
    string img_name = orifile_path + m_cid + ".png";

    //cout << "img_name : "<< img_name << endl;
    imwrite(img_name, original);
    // 이부분에서 추가
    // 저장 사진과 현재 matadata_queue.front().BGR_frame를 비교 

    matadata_queue.front().image_save_state = true;

    original.release();
}

void Media_cls::convert_frames2gray(queue<matadata>& matadata_queue) {
    Mat original;
    matadata_queue.front().BGR_frame.copyTo(original);

    cvtColor(original, m_G_frame, COLOR_BGR2GRAY);

    matadata_queue.front().G_frame_state = true;

    original.release();
}

void Media_cls::edge_detection_BGR(queue<matadata>& matadata_queue) {
    
    Canny(m_G_frame, m_edge_result, 20, 40);

    matadata_queue.front().feature_vector_state = true;
}

void Media_cls::capture_frame_task(queue<matadata>& matadata_queue, mutex& matadata_mutex) {
    pthread_setname_np(pthread_self(), "thread 0");
    while (true) {
        if (set_frame() == -1) {
            cerr << "Error: init_frame failed." << endl;
        }
        capture_frame(matadata_queue, matadata_mutex);
        cout << "    matadata_queue size : " << matadata_queue.size() << endl;
        this_thread::sleep_for(chrono::milliseconds(800));
    }
}

void Media_cls::start_capture_thread(queue<matadata>& matadata_queue, mutex& matadata_mutex) {
    capture_thread = thread(&Media_cls::capture_frame_task, this, ref(matadata_queue), ref(matadata_mutex));
}

void Media_cls::capture_image_save_task(std::queue<matadata>& matadata_queue) {
    pthread_setname_np(pthread_self(), "thread 1");

    while (true) {
        if (!matadata_queue.empty()) {
            if (!matadata_queue.front().cid.empty() && matadata_queue.front().image_save_state == false) {
                auto total_start_time = std::chrono::steady_clock::now();
                image_save(matadata_queue);
                auto total_end_time = std::chrono::steady_clock::now();

                int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
                spdlog::info("T1: {} ms", duration);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Media_cls::start_capture_save_thread(queue<matadata>& matadata_queue) {
    image_save_thread = thread(&Media_cls::capture_image_save_task, this, ref(matadata_queue));
}

void Media_cls::convert_frames2gray_task(queue<matadata>& matadata_queue) {
    pthread_setname_np(pthread_self(), "thread 2");

    while (true) {
        if (!matadata_queue.empty()) {
            if (!matadata_queue.front().cid.empty() && matadata_queue.front().G_frame_state == false) {
                auto total_start_time = std::chrono::steady_clock::now();
                convert_frames2gray(matadata_queue);
                auto total_end_time = std::chrono::steady_clock::now();

                int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
                spdlog::info("T2: {} ms", duration);

                std::cout << "T2: " << duration << " ms" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Media_cls::start_convert_frames2gray_thread(queue<matadata>& matadata_queue) {
    convert_thread = thread(&Media_cls::convert_frames2gray_task, this, ref(matadata_queue));
}

void Media_cls::edge_detection_task(queue<matadata>& matadata_queue) {
    pthread_setname_np(pthread_self(), "thread 4");

    while (true) {
        if (!matadata_queue.empty()) {
            if (matadata_queue.front().G_frame_state == true && matadata_queue.front().feature_vector_state == false) {
                auto total_start_time = std::chrono::steady_clock::now();
                edge_detection_BGR(matadata_queue);
                auto total_end_time = std::chrono::steady_clock::now();

                int duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_end_time - total_start_time).count();
                spdlog::info("T4: {} ms", duration);

                std::cout << "T4: " << duration << " ms" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}


void Media_cls::start_edge_detection_thread(queue<matadata>& matadata_queue) {
    edge_thread = thread(&Media_cls::edge_detection_task, this, ref(matadata_queue));
}
