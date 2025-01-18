#include "endo_viewer.h"
#include <ctime>
#include "./inc/v4l2_capture.h"

#define DO_EFFECIENCY_TEST 1

namespace {

    std::chrono::steady_clock::time_point getCurrentTimePoint() {
        return ::std::chrono::steady_clock::now();
    }

    long getDurationSince(const std::chrono::steady_clock::time_point &start_time_point)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		return ::std::chrono::duration_cast<
            std::chrono::milliseconds>(now - start_time_point).count();
    }

    inline ::std::string getCurrentTimeStr()
    {
        time_t timep;
        time(&timep);
        char tmp[64];
        strftime(tmp, sizeof(tmp), "%Y%m%d_%H%M%S", localtime(&timep));

        return std::string(tmp);
    }

    const uint8_t TIME_INTTERVAL = 17;
}


EndoViewer::EndoViewer() 
    : imwidth(1920), imheight(1080)
    , _image_l(cv::Mat(imheight, imwidth, CV_8UC3))
    , _image_r(cv::Mat(imheight, imwidth, CV_8UC3))
    , _is_write_to_video(false)
{
}


EndoViewer::~EndoViewer() {
    delete _cap_l;
    delete _cap_r;
    cv::destroyAllWindows();
}


void EndoViewer::startup(uint8_t left_cam_id, uint8_t right_cam_id, bool is_write_to_video) {
    _is_write_to_video = is_write_to_video;
    if(_is_write_to_video) {
        _thread_writer = std::thread(&EndoViewer::writeVideo, this);
        _thread_writer.detach();
    }

    _thread_read_l = std::thread(&EndoViewer::readLeftImage, this, left_cam_id);
    _thread_read_l.detach();
    _thread_read_r = std::thread(&EndoViewer::readRightImage, this, right_cam_id);
    _thread_read_r.detach();

    show();
}


void EndoViewer::readLeftImage(int index) {
    _cap_l = new V4L2Capture(imwidth, imheight, 3);
    while(!_cap_l->openDevice(index)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("Camera %d is retrying to connection!!!\n", index);
    }

    bool flag = 0;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = _cap_l->ioctlDequeueBuffers(_image_l.data);
        flag = flag && (!_image_l.empty());
        if(!flag) {
            printf("EndoViewer::readLeftImage: USB ID: %d, image empty: %d.\n",
                    index, _image_l.empty());
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

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
    _cap_r = new V4L2Capture(imwidth, imheight, 3);
    while(!_cap_r->openDevice(index)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        printf("Camera %d is retrying to connection!!!\n", index);
    }

    bool flag = 0;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = _cap_r->ioctlDequeueBuffers(_image_r.data);
        flag = flag && (!_image_r.empty());
        if(!flag) {
            printf("EndoViewer::readRightImage: USB ID: %d, image empty: %d.\n",
                    index, _image_r.empty());
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

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

    cv::Mat imleft, imright;
    bool is_show_left = true;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        cv::cvtColor(_image_l, imleft, cv::COLOR_RGB2BGR);
        cv::cvtColor(_image_r, imright, cv::COLOR_RGB2BGR);
        cv::hconcat(imleft, imright, bino);
        cv::imshow(win_name, bino); 
        if(is_show_left) {
            cv::imshow(win_name2, imleft);
        }
        else {
            cv::imshow(win_name2, imright);
        }
        char key = cv::waitKey(10);
        if(key == 'q') {
            printf("EndoViewer: exit video showing.\n");
            break;
        }
        if(key == 'c') {
            is_show_left = !is_show_left;
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


void EndoViewer::writeVideo() {
    cv::Size size = cv::Size(imwidth * 2, imheight);
    _writer.open(getCurrentTimeStr() + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, size, true);
    if (!_writer.isOpened()) {
        std::cout << "EndoViewer: cannot open the video writer!\n";
        std::exit(-1);
    }

    cv::Mat bino;
    bool is_show_left = true;
    auto time_org = ::getCurrentTimePoint();
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        cv::hconcat(_image_l, _image_r, bino);
        _writer.write(bino);

        auto ms = getDurationSince(time_start);

        if(getDurationSince(time_org) > (60*1000)) {
            _writer.release();
            _writer.open(getCurrentTimeStr() + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, size, true);
            time_org = ::getCurrentTimePoint();
        }
#if DO_EFFECIENCY_TEST
        printf("EndoViewer::writeVideo: [%ld]ms elapsed.\n", ms);
#endif

        if(ms < 17) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_INTTERVAL - ms));
        }
    }
}