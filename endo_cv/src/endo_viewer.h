#ifndef H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64
#define H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64
#include <thread>
#include <cstdint>
#include <opencv2/opencv.hpp>

class EndoViewer {
public:
    EndoViewer();
    ~EndoViewer();

    void startup(uint8_t left_cam_id = 0, uint8_t right_cam_id = 1);

    const uint16_t imwidth;
    const uint16_t imheight;
private:
    void readLeftImage(int index);
    void readRightImage(int index);
    void show(); // OpenCV can only show window in the same thread


    std::thread _thread_read_l;
    std::thread _thread_read_r;

    cv::VideoCapture _cap_l;
    cv::VideoCapture _cap_r;

    cv::Mat _image_l;
    cv::Mat _image_r;
};

#endif /* H_WLF_C5AA0CDA_9668_4C6C_B6F9_9EEFE7292C64 */
