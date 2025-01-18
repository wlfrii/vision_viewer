#ifndef H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64
#define H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64
#include <thread>
#include <cstdint>
#include <opencv2/opencv.hpp>

class V4L2Capture;

class BinoViewer {
public:
    BinoViewer(uint16_t imwidth, uint16_t imheight);
    ~BinoViewer();

    void startup(uint8_t left_cam_id = 0, uint8_t right_cam_id = 1, bool is_write_to_video = false);

    const uint16_t imwidth;
    const uint16_t imheight;
private:
    void readImage(bool is_right);
    void show(); // OpenCV can only show window in the same thread
    void writeVideo();

    std::thread _thread_read[2];
    cv::VideoCapture _cap[2];
    cv::Mat _image[2];
    uint8_t _index[2];

    bool _is_write_to_video;
    cv::VideoWriter  _writer;
    std::thread _thread_writer;
};

#endif /* H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64 */
