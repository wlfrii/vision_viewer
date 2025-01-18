#ifndef H_WLF_D403E4B6_47E3_41E3_8C3D_0AC15E6E415B
#define H_WLF_D403E4B6_47E3_41E3_8C3D_0AC15E6E415B
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Supported screen resolution.
 */
enum ScreenResolution : uint8_t {
    SCREEN_1920_1080,
    SCREEN_2560_1440,
    SCREEN_5120_1440,
    SCREEN_1920_1920,
    SCREEN_2560_2560, 
    DISPLAY_DEVICE_NUM
};

/**
 * @brief Get the desccription of the display screen.
 * 
 * @param screen The given screen.
 * @return const std::string& 
 */
const std::string& getDesc(const ScreenResolution& resolution);

/**
 * @brief Get the screen resolution.
 * 
 * @param screen The given screen.
 * @param width  The width of the screen.
 * @param height The height of the screen.
 */
const void getResolution(const ScreenResolution& screen, 
                         uint16_t& width, uint16_t& height);

/**
 * @brief The settable options for VisionViewer.
 */
struct DisplayScreen {
    DisplayScreen();

    const char* c_str() const;

    ScreenResolution resolution;///< The screen specified for displaying.
    int x_devia;                ///< Specified the x-deviation of the screen.    
    bool is_3d;                 ///< Specify whether display 3D.
    bool is_hcompress;          ///< Specify H-compression in 3D display, false is default.
};

/**
 * @brief Parse Screen information
 * 
 * @param argstr The input arguments from main().
 * @param screens The parsed screen information.
 * @return 
 *   @retval true For parsed successfully.
 *   @retval false For failed.
 */
bool parseScreenInfo(std::string argstr, std::vector<DisplayScreen>& screens);

/**
 * @brief Print description of screen argument.
 */
void printScreenArgDesc();

/**
 * @brief The settable options for VisionViewer.
 */
struct CameraViewerOption {
    bool        is_mono;        ///< Specify the monocular or binocular.
    uint8_t     index[2];       ///< Specify the camera index.
    uint16_t    imwidth;        ///< Image width, required in camera mode.
    uint16_t    imheight;       ///< Image height, required in camera mode.
    
    std::vector<DisplayScreen> screens; ///< Display screens.
};

/**
 * @brief The settable options for VisionViewer.
 */
struct VideoViewerOption {
    VideoViewerOption();

    std::string video_path;     ///< The path of the video to be shown.
    bool        is_mono;        ///< Specify the video is from monocular or binocular.
    bool        is_looped;      ///< Specify to loop the displaying, looping is default.
    bool        is_bgr;         ///< Sepcify the video color pattern, RGB is default.
    int         interval;       ///< Specify the refresh interval in milliseconds.

    std::vector<DisplayScreen> screens; ///< Display screens.
};

#endif /* H_WLF_D403E4B6_47E3_41E3_8C3D_0AC15E6E415B */
