#include "vision_viewer.h"
#include <thread>
#include <stdexcept>

#define DO_EFFECIENCY_TEST 0

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

    inline std::string getCurrentTimeStr() {
        time_t timep;
        time(&timep);
        char tmp[64];
        strftime(tmp, sizeof(tmp), "%Y%m%d_%H%M%S", localtime(&timep));

        return std::string(tmp);
    }

    const uint8_t TIME_INTTERVAL = 17;
}


VisionViewer::VisionViewer(const VideoViewerOption& option) 
    : _mode(VIDEO)
    , _vid_option(option)
    , _should_stop(false)
    , _should_write(false) {
}

VisionViewer::VisionViewer(const CameraViewerOption& option) 
    : _mode(CAMERA)
    , _cam_option(option)
    , _should_stop(false) 
    , _should_write(false){
}

VisionViewer::~VisionViewer() {
    cv::destroyAllWindows();
}

void VisionViewer::startShow() {
    // Parse screen info
    parseDisplayInfo();

    if(_mode == VIDEO) {
        std::thread read_thread;
        read_thread = std::thread(&VisionViewer::readVideoFrame, this);
        read_thread.detach();
    }
    else {
        if(_cam_option.is_mono) {
            std::thread read_thread;
            read_thread = std::thread(&VisionViewer::readCameraFrame, this, 0);
            read_thread.detach();
        }
        else {
            std::thread read_thread_l;
            read_thread_l = std::thread(&VisionViewer::readCameraFrame, this, 0);
            read_thread_l.detach();
            std::thread read_thread_r;
            read_thread_r = std::thread(&VisionViewer::readCameraFrame, this, 1);
            read_thread_r.detach();
        }
    }    

    std::thread write_thread;
    write_thread = std::thread(&VisionViewer::writeVideo, this);
    write_thread.detach();

    show();
}

void VisionViewer::parseDisplayInfo() {
    _win_names_2d.clear();
    _win_info_3d.clear();
    
    const auto& screens = _mode == VIDEO ? _vid_option.screens : _cam_option.screens;

    int count2d = 0, count3d = 0;
    // Initialize all display screens
    for(const auto& screen : screens) {
        cvWinInfo info;
        if(screen.is_3d) {
            info.is_hcompress = screen.is_hcompress;
            getResolution(screen.resolution, info.win_width, info.win_height);
            
            info.win_name = "Win3D-" + std::to_string(count3d++);
            _win_info_3d.push_back(info);
        }
        else {
            info.win_name = "Win2D-" + std::to_string(count2d++);
            _win_names_2d.push_back(info.win_name);
        }
            
        cv::namedWindow(info.win_name, cv::WINDOW_NORMAL);
        cv::moveWindow(info.win_name, screen.x_devia, 0);
        cv::setWindowProperty(info.win_name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
    }

    if(_win_names_2d.size() == 0 && _win_info_3d.size() == 0) {
        printf("VisionViewer: no screen is specified, app exit.\n");
        std::exit(-1);
    }
    printf("VisionViewer: [%ld] 2D display and [%ld] 3D display are initialized.\n",
        _win_names_2d.size(), _win_info_3d.size());
}

void VisionViewer::readVideoFrame() {
    auto& option = _vid_option;
    auto& cap = _capture[0];
    auto& tri_frame_prop = _tri_frame_prop[0];

    cap = cv::VideoCapture(option.video_path);
    if(!cap.isOpened()) {
        std::ostringstream err;
        err << "VisionViewer: Unable to open input video file: " 
            << option.video_path << std::endl;
        throw std::invalid_argument(err.str());
    }

    _imwidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    _imheight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    printf("Video property: %d x %d resolution, with %f FPS\n", _imwidth, _imheight, fps);    

    if(option.interval == 0) {
        option.interval = (int)1000 / fps;
        printf("      no extra refresh interval is specified, 1/FPS=%d is used.\n",
                option.interval);
    }

    size_t loop_count = 0;
    uint8_t idx = 0;
    cv::Mat frame;
    while(!_should_stop) {
        auto time_start = getCurrentTimePoint();

        bool flag = cap.read(frame);
        if(!flag) {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            if(option.is_looped) {
                printf("VisionViewer: loop displaying count [%ld]\n", ++loop_count);
                continue;
            }
            else {
                _should_stop = true;
                _sem_show.release();
                break;
            }
        }

        // Convert color if needed.
        if(option.is_bgr) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        }

        idx = tri_frame_prop.getOldestIndex();
        if(option.is_mono) {
            _frames[0][idx] = frame.clone();
        }
        else {
            _frames[0][idx] = frame.colRange(0, _imwidth / 2);
            _frames[1][idx] = frame.colRange(_imwidth / 2, _imwidth);
        }
        _tri_frame_prop[0].update(idx);
        _tri_frame_prop[1].update(idx);
        
        auto delta_ms = option.interval - getDurationSince(time_start);
#if DO_EFFECIENCY_TEST
        printf("VisionViewer::readVideoFrame: [%ld]ms elapsed.\n", ms);
#endif
        if(delta_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms));
        }
        _sem_show.release();
        // if(_should_write) {
        //     _sem_write.release();
        // }
    }
}

void VisionViewer::readCameraFrame(bool is_right) {
    auto initVideoCapture = [](cv::VideoCapture &capture, int cam_id, int w, int h) {
        capture.open(cam_id);
        capture.set(cv::CAP_PROP_FOURCC, cv::CAP_OPENCV_MJPEG);
        capture.set(cv::CAP_PROP_FRAME_WIDTH, w);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, h);
        capture.set(cv::CAP_PROP_BUFFERSIZE, 3);
        // capture.set(cv::CAP_PROP_SHARPNESS, 3);
    };
    auto& option = _cam_option;
    auto& cap = _capture[is_right];
    auto& frames = _frames[is_right];
    auto cam_id = option.index[is_right];
    auto& tri_frame_prop = _tri_frame_prop[is_right];

    _imwidth = _cam_option.imwidth;
    _imheight = _cam_option.imheight;

    initVideoCapture(cap, cam_id, _imwidth, _imheight);

    bool flag = 0;
    uint8_t idx = 0;
    cv::Mat frame;
    while(true) {
        auto time_start = ::getCurrentTimePoint();

        flag = cap.read(frame);
        flag = flag && (!frame.empty());
        if(!flag) {
            printf("VisionViewer::read%sImage: USB ID: %d, image empty: %d.\n",
                    is_right ? "Right":"Left", cam_id, frame.empty());
            cap.release();
            initVideoCapture(cap, cam_id, _imwidth, _imheight);
            std::this_thread::sleep_for(std::chrono::seconds(500));
            continue;
        }
        
        idx = tri_frame_prop.getOldestIndex();
        frames[idx] = frame.clone();
        tri_frame_prop.update(idx);

        auto delta_ms = getDurationSince(time_start) - TIME_INTTERVAL;
#if DO_EFFECIENCY_TEST
        printf("BinoViewer::read%sImage: [%ld]ms elapsed.\n", 
                is_right ? "Right":"Left", ms);
#endif
        if(delta_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms));
        }
        _sem_show.release();
    }
}


void VisionViewer::show() {
    bool has_2d = _win_names_2d.size() > 0;
    bool has_3d = _win_info_3d.size() > 0;
    bool is_mono = _mode == VIDEO ? _vid_option.is_mono : _cam_option.is_mono;
    bool is_show_right = false;

    uint8_t idx = 0;
    cv::Mat image, imleft, imright;
    
    while(!_should_stop) {
        _sem_show.take();

        idx = _tri_frame_prop[0].getNewestIndex();
        imleft = _frames[0][idx].clone();
        idx = _tri_frame_prop[1].getNewestIndex();
        imright = _frames[1][idx].clone();

        // Display 3D
        if(!is_mono && has_3d) {
            for(auto& win_info : _win_info_3d) {
                int win_width = win_info.win_width;
                int win_height = win_info.win_height;
                float scale = 1.f * win_height / _imheight;
                int new_width = round(_imwidth * scale);
                int x_half_devia = (win_width - new_width) / 2;;
                cv::Size cvsize = cv::Size(new_width, win_height);

                // printf("win size:%dx%d, scale:%f, x_half_devia:%d\n",
                //     win_width, win_height, scale, x_half_devia);

                cv::Mat imbino = cv::Mat(win_height, win_width*2, CV_8UC3, cv::Scalar(0, 0, 0));
                int start = x_half_devia;
                cv::resize(imleft, imbino.colRange(start, start + new_width), cvsize);
                start = x_half_devia + win_width;
                cv::resize(imright, imbino.colRange(start, start + new_width), cvsize);

                cv::imshow(win_info.win_name, imbino);
            }
        }

        // Display 2D
        if(has_2d) {
            image = is_show_right ? imright : imleft;

            for(auto& win_name : _win_names_2d) {
                cv::imshow(win_name, image);
            }
        }

        char key = cv::waitKey(10);
        if(key == 'q') {
            printf("VisionViewer: exit video showing.\n");
            _should_stop = true;
            break;
        }
        else if(key == 'c') {
            if(!is_mono) {
                is_show_right = !is_show_right;
            }
        }
        else if(key == 'p') {
            std::string prefix = getCurrentTimeStr();
            if(is_mono) {
                cv::imwrite(prefix + ".bmp", imleft);
                printf("VisionViewer: save mono image %s done.\n", prefix.c_str());
            }
            else {
                cv::Mat imbino;
                cv::hconcat(imleft, imright, imbino);
                cv::imwrite(prefix + ".bmp", imbino);
                printf("VisionViewer: save bino image %s done.\n", prefix.c_str());
            }            
        }
        else if(key == 's') {
            _should_write = !_should_write;
            if(!refreshVideoWriter()) {
                _should_write = false;
            }
        }

        if(_should_write) {
            _sem_write.release();
        }

#if DO_EFFECIENCY_TEST
        printf("VisionViewer::show2D: [%ld]ms elapsed.\n", ms);
#endif
    }
}

void VisionViewer::writeVideo() {
    bool is_mono = _mode == VIDEO ? _vid_option.is_mono : _cam_option.is_mono;
    cv::Mat image(_imheight, is_mono ? _imwidth : _imwidth*2, CV_8UC3, cv::Scalar(0, 0, 0));

    int max_minutes = 5;
    uint8_t idx = 0;
    while(true) {
        _sem_write.take();

        if(is_mono) {
            idx = _tri_frame_prop[0].getNewestIndex();
            image = _frames[0][idx].clone();
        }
        else {
            idx = _tri_frame_prop[0].getNewestIndex();
            _frames[0][idx].copyTo(image.colRange(0, _imwidth));
            idx = _tri_frame_prop[1].getNewestIndex();
            _frames[1][idx].copyTo(image.colRange(_imwidth, 2*_imwidth));
        }
        _video_writer.write(image); 

        if(getDurationSince(_write_start) > (max_minutes*60*1000)) {
            _video_writer.release();
            printf("VisionViewer: write out video is beyond %d minutes, a new write out "
                "process is started.\n", max_minutes);
            refreshVideoWriter();
        }
#if DO_EFFECIENCY_TEST
        printf("VisionViewer::writeVideo: [%ld]ms elapsed.\n", ms);
#endif
    }
}

bool VisionViewer::refreshVideoWriter() {
    if(_should_write) {
        bool is_mono = _mode == VIDEO ? _vid_option.is_mono : _cam_option.is_mono;
        cv::Size size = cv::Size(is_mono ? _imwidth : _imwidth*2, _imheight);
        std::string video_name = getCurrentTimeStr() + ".avi";
        _video_writer.open(video_name,  cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                           30, size, true);
        if (!_video_writer.isOpened()) {
            std::cout << "VisionViewer: cannot open the video writer, writer thread stop!\n";
            return false;
        }
        _write_start = ::getCurrentTimePoint();
        printf("VisionViewer: start write video to %s, with image size: %dx%d.\n", 
            video_name.c_str(), size.width, size.height);
    }
    else {
        _video_writer.release();
        printf("VisionViewer: stop write video.\n");
    }
    return true;
}