#include "endo_viewer.h"

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

    const uint8_t TIME_INTTERVAL = 17;
}


EndoViewer::EndoViewer() 
    : imwidth(1920), imheight(1080)
    , _image_l(cv::Mat(imheight, imwidth, CV_8UC3))
    , _image_r(cv::Mat(imheight, imwidth, CV_8UC3))
{
}


EndoViewer::~EndoViewer() {
    cv::destroyAllWindows();
}


void EndoViewer::startup(uint8_t left_cam_id, uint8_t right_cam_id) {
    _thread_read_l = std::thread(&EndoViewer::readLeftImage, this, left_cam_id);
    _thread_read_l.detach();
    _thread_read_r = std::thread(&EndoViewer::readRightImage, this, right_cam_id);
    _thread_read_r.detach();

    show();
}


void EndoViewer::readLeftImage(int index) {
    ::initVideoCapture(_cap_l, index, imwidth, imheight);

    cv::Mat image;
    bool flag = 0;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = _cap_l.read(image);
        flag = flag && (!image.empty());
        if(!flag) {
            printf("EndoViewer::readLeftImage: USB ID: %d, image empty: %d.\n",
                    index, image.empty());
            _cap_l.release();
            ::initVideoCapture(_cap_l, index, imwidth, imheight);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        _image_l = image.clone();   

        auto ms = getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("EndoViewer::readLeftImage: [%ld]ms elapsed.\n", ms);
#endif
        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}


void EndoViewer::readRightImage(int index) {
    ::initVideoCapture(_cap_r, index, imwidth, imheight);

    cv::Mat image;
    bool flag = 0;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = _cap_r.read(image);
        flag = flag && (!image.empty());
        if(!flag) {
            printf("EndoViewer::readRightImage: USB ID: %d, image empty: %d.\n",
                    index, image.empty());
            _cap_r.release();
            ::initVideoCapture(_cap_r, index, imwidth, imheight);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        _image_r = image.clone();   

        auto ms = getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("EndoViewer::readRightImage: [%ld]ms elapsed.\n", ms);
#endif
        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}   


void EndoViewer::show() {
    cv::Mat bino;
    std::string win_name = "Bino";
    cv::namedWindow(win_name, cv::WINDOW_NORMAL);
    cv::moveWindow(win_name, 1920, 0);
    cv::setWindowProperty(win_name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    std::string win_name2 = "Mono";
    cv::namedWindow(win_name2, cv::WINDOW_NORMAL);
    cv::moveWindow(win_name2, 0, 0);
    cv::setWindowProperty(win_name2, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

    while(true) {
        auto time_start = ::getCurrentTimePoint();

        cv::hconcat(_image_l, _image_r, bino);
        cv::imshow(win_name, bino); cv::imshow(win_name2, _image_l);
        char key = cv::waitKey(10);
        if(key == 'q') {
            std::exit(1);
        }

        auto ms = getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("EndoViewer::showBino: [%ld]ms elapsed.\n", ms);
#endif
        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}