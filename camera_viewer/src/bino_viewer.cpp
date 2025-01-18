#include "bino_viewer.h"
#include <ctime>

#define DO_EFFECIENCY_TEST 0

namespace {
    void initVideoCapture(cv::VideoCapture &capture, int index, int width, int height) {
        capture.open(index);
        capture.set(cv::CAP_PROP_FOURCC, cv::CAP_OPENCV_MJPEG);
        capture.set(cv::CAP_PROP_FRAME_WIDTH, width);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
        capture.set(cv::CAP_PROP_BUFFERSIZE, 3);
        // capture.set(cv::CAP_PROP_SHARPNESS, 3);
    }

    std::chrono::steady_clock::time_point getCurrentTimePoint() {
        return ::std::chrono::steady_clock::now();
    }

    long getDurationSince(const std::chrono::steady_clock::time_point &start_time_point)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		return ::std::chrono::duration_cast<
            std::chrono::milliseconds>(now - start_time_point).count();
    }

    inline std::string getCurrentTimeStr()
    {
        time_t timep;
        time(&timep);
        char tmp[64];
        strftime(tmp, sizeof(tmp), "%Y%m%d_%H%M%S", localtime(&timep));

        return std::string(tmp);
    }

    const uint8_t TIME_INTTERVAL = 17;

    const uint16_t GOOVIS_WIDTH = 2560;
    const uint16_t GOOVIS_HEIGHT = 1440;
}


BinoViewer::BinoViewer(uint16_t imwidth, uint16_t imheight) 
    : imwidth(imwidth), imheight(imheight)
    , _is_write_to_video(false) {
    _image[0] = cv::Mat(imheight, imwidth, CV_8UC3);
    _image[1] = cv::Mat(imheight, imwidth, CV_8UC3);
}


BinoViewer::~BinoViewer() {
    cv::destroyAllWindows();
}

volatile bool video_write_out = false;

void BinoViewer::startup(uint8_t left_cam_id, uint8_t right_cam_id, bool is_write_to_video) {
    // _is_write_to_video = is_write_to_video;
    // if(_is_write_to_video) {
        _thread_writer = std::thread(&BinoViewer::writeVideo, this);
        _thread_writer.detach();
        printf("Start video write thread.\n");
    // }

    _index[0] = left_cam_id;
    _index[1] = right_cam_id;
    for(int i = 0; i < 1; i++) {
        _thread_read[i] = std::thread(&BinoViewer::readImage, this, i);
        _thread_read[i].detach();
    }

    show();
}


void BinoViewer::readImage(bool is_right) {
    auto& cap = _cap[is_right];
    auto& image = _image[is_right];
    auto index = _index[is_right];

    initVideoCapture(cap, index, imwidth, imheight);

    bool flag = 0;
    cv::Mat imtemp;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = cap.read(imtemp);
        flag = flag && (!imtemp.empty());
        if(!flag) {
            printf("BinoViewer::read%sImage: USB ID: %d, image empty: %d.\n",
                    is_right ? "Right":"Left", index, imtemp.empty());
            cap.release();
            initVideoCapture(cap, index, imwidth, imheight);
            std::this_thread::sleep_for(std::chrono::seconds(500));
            continue;
        }
        if((!imtemp.empty())) {
            image = imtemp.clone();
        }

        auto ms = getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("BinoViewer::read%sImage: [%ld]ms elapsed.\n", 
                is_right ? "Right":"Left", ms);
#endif
        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}

// #define ENABLE_GOOVIS_SHOW

void BinoViewer::show() {
#ifdef ENABLE_GOOVIS_SHOW
    std::string win_name = "Bino";
    cv::namedWindow(win_name, cv::WINDOW_NORMAL);
    cv::moveWindow(win_name, 1920, 0);
    cv::setWindowProperty(win_name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
#endif // ENABLE_GOOVIS_SHOW
    std::string win_name2 = "Mono";
    cv::namedWindow(win_name2, cv::WINDOW_NORMAL);
    cv::moveWindow(win_name2, 0, 0);
    cv::setWindowProperty(win_name2, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    cv::Mat imbino = cv::Mat(GOOVIS_HEIGHT, GOOVIS_WIDTH*2, CV_8UC3);
    cv::Mat imgoovis = cv::Mat(GOOVIS_HEIGHT, GOOVIS_WIDTH, CV_8UC3, 
                                cv::Scalar(255, 255, 255));
    cv::Mat imleft = imgoovis, imright = imgoovis;

    float scale = GOOVIS_HEIGHT / imheight;
    int new_width = round(imwidth * scale);
    int x_half_devia = (GOOVIS_WIDTH - new_width) / 2;;
    cv::Size cvsize = cv::Size(new_width, GOOVIS_HEIGHT);

    bool is_show_left = true;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        imleft = _image[0].clone();
        imright = _image[1].clone();
        
        int start = x_half_devia;
        cv::resize(imleft, imbino.colRange(start, start + new_width), cvsize);
        start = x_half_devia + GOOVIS_WIDTH;
        cv::resize(imright, imbino.colRange(start, start + new_width), cvsize);
#ifdef ENABLE_GOOVIS_SHOW
        cv::resize(imbino, imgoovis, cv::Size(GOOVIS_WIDTH, GOOVIS_HEIGHT));
        cv::imshow(win_name, imgoovis); 
#endif // ENABLE_GOOVIS_SHOW
        if(is_show_left) {
            cv::line(imleft, cv::Point2i(0, imheight/2), cv::Point2i(imwidth, imheight/2), 
                cv::Scalar(0, 255, 255));
            cv::line(imleft, cv::Point2i(imwidth/2, 0), cv::Point2i(imwidth/2, imheight), 
                cv::Scalar(0, 255, 255));
            cv::imshow(win_name2, imleft);
        }
        else {
            cv::imshow(win_name2, imright);
        }
        // cv::imshow(win_name2, imbino);
        char key = cv::waitKey(10);
        if(key == 'q') {
            printf("BinoViewer: exit video showing.\n");
            break;
        }
        if(key == 'c') {
            is_show_left = !is_show_left;
        }
        if(key == 'p') {
            std::string prefix = getCurrentTimeStr();
            cv::imwrite(prefix + "_L.bmp", imleft);
            cv::imwrite(prefix + "_R.bmp", imright);
            printf("Save bino image %s done.\n", prefix.c_str());
        }
        if(key == 'w') {
            video_write_out = !video_write_out;
            printf("%s write video.\n", video_write_out ? "start" : "stop");
        }

        auto ms = getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("BinoViewer::showBino: [%ld]ms elapsed.\n", ms);
#endif
        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}


void BinoViewer::writeVideo() {
    cv::Size size = cv::Size(imwidth, imheight);
    _writer.open(getCurrentTimeStr() + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, size, true);
    if (!_writer.isOpened()) {
        std::cout << "BinoViewer: cannot open the video writer!\n";
        std::exit(-1);
    }

    cv::Mat bino;
    bool is_show_left = true;
    auto time_org = ::getCurrentTimePoint();
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        if(video_write_out) {
            //cv::hconcat(_image[0], _image[1], bino);
            _writer.write(_image[0]);

            if(getDurationSince(time_org) > (60*3000)) {
                _writer.release();
                _writer.open(getCurrentTimeStr() + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, size, true);
                time_org = ::getCurrentTimePoint();
            }
#if DO_EFFECIENCY_TEST
        printf("BinoViewer::writeVideo: [%ld]ms elapsed.\n", ms);
#endif
        }
        int delta_ms = 33 - getDurationSince(time_start);
        if(delta_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms));
        }
    }
}