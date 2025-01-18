/**
 * @file vision_viewer.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-03-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef H_WLF_D574CEFC_5F34_4383_A1E9_C2720631993D
#define H_WLF_D574CEFC_5F34_4383_A1E9_C2720631993D
#include <cstdint>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <chrono>
#include "./define/triple_buffer.h"
#include "./define/vision_options.h"
#include "./define/csemaphore.h"

/**
 * @brief A class for viewing monocular or binocular video, based on OpenCV.
 */
class VisionViewer {
public:
    /**
     * @brief Construct a new Video Viewer object.
     * 
     * @param option The VisionViewerOption object.
     */
    VisionViewer(const VideoViewerOption& option);

    /**
     * @brief Construct a new Video Viewer object.
     * 
     * @param option The CameraViewerOption object.
     */
    VisionViewer(const CameraViewerOption& option);

    /**
     * @brief Destroy the Video Viewer object.
     */
    ~VisionViewer();

    // void show(const std::string& video_path, bool is_mono,
    //           bool islooped, uint8_t refresh_interval, 
    //           bool is_bgr_fmt, DisplayDevice device);

    // void showBoth(const std::string& video_path, uint8_t refresh_interval = 17, 
    //           bool is_bgr_fmt = false);

    void startShow();

private:
    /**
     * @brief OpenCV window info.
     */
    struct cvWinInfo {
        std::string win_name;       ///< The window name.
        uint16_t    win_width;      ///< The window width.
        uint16_t    win_height;     ///< The window height.
        bool        is_hcompress;   ///< Has h-compress in 3D display.
    };

    /**
     * @brief Parse the display information.
     */
    void parseDisplayInfo();

    /**
     * @brief Do frame rading.
     */
    void readVideoFrame();

    /**
     * @brief Do frame rading.
     */
    void readCameraFrame(bool is_right);

    /**
     * @brief Show.
     */
    void show();

    /**
     * @brief Write video.
     */
    void writeVideo();

    /**
     * @brief Refresh video writer.
     */
    bool refreshVideoWriter();

private:
    /**
     * @brief Specify the running mode.
     */
    enum Mode{ VIDEO, CAMERA };
    Mode _mode; ///< The running mode.
    
    uint16_t _imwidth;                  ///< Image width.
    uint16_t _imheight;                 ///< Image height.

    VideoViewerOption  _vid_option;     ///< The VisionViewerOptions object.
    CameraViewerOption _cam_option;     ///< The VisionViewerOptions object.

    std::vector<std::string> _win_names_2d; ///< The necessary information for 2D display.
    std::vector<cvWinInfo>   _win_info_3d;  ///< The necessary information for 3D display.

    cv::VideoCapture _capture[2];       ///< OpenCV capture for camera or video capture.
    volatile bool    _should_stop;      ///< Flag for controlling stop.
    cv::VideoWriter  _video_writer;     ///< For video write out.
    volatile bool    _should_write;     ///< Flag for write out video.
    std::chrono::steady_clock::time_point _write_start;   ///< The start write time.

    CSemaphore _sem_show;                     ///< Semaphore for control display.
    CSemaphore _sem_write;                    ///< Semaphore for control write out.

    TripleBuffer<uint8_t> _tri_frame_prop[2]; ///< For read frames.
    cv::Mat _frames[2][3];                    ///< For store frames.

};

#endif /* H_WLF_D574CEFC_5F34_4383_A1E9_C2720631993D */
