#include "vision_options.h"
#include <sstream>

const std::string& getDesc(const ScreenResolution& resolution) {
    static std::vector<std::string> desc = {
        "SCREEN_1920_1080",
        "SCREEN_2560_1440",
        "SCREEN_5120_1440",
        "SCREEN_1920_1920",
        "SCREEN_2560_2560" ,
        "NO_SCREEN"
    };
    return desc[resolution];
}

const void getResolution(const ScreenResolution& resolution, 
                         uint16_t& width, uint16_t& height) {
    switch (resolution)
    {
    case SCREEN_1920_1080:
        width = 1920;
        height = 1080;
        break;
    case SCREEN_2560_1440:
        width = 2560;
        height = 1440;
        break;
    case SCREEN_5120_1440:
        width = 5120;
        height = 1440;
        break;
    case SCREEN_1920_1920:
        width = 1920;
        height = 1920;
        break;
    case SCREEN_2560_2560:
        width = 2560;
        height = 2560;
        break;
    default:
        width = 0;
        height = 0;
        break;
    }
}

bool parseScreenInfo(std::string argstr, std::vector<DisplayScreen>& screens) {
    for(auto& ch : argstr) {
        if(ch == ',') {
            ch = ' ';
        }
    }

    int value[4] = {0, 0, 0, 0};
    int count = 0;
    std::stringstream ss(argstr);
    while(ss >> value[count] && count < 4) {
        count++; 
    }

    if(count < 1) {
        return false;
    }

    DisplayScreen screen;
    screen.resolution = ScreenResolution(value[0]);
    screen.x_devia = value[1];
    screen.is_3d = value[2];
    screen.is_hcompress = value[3];
    printf("VisionViewer: specify screen with info: %s\n", screen.c_str());
    screens.push_back(screen);

    return true;
}

void printScreenArgDesc() {
    printf("\t\t -n [\"x_devia resolution 3d hcompress\"]\tSpecify the displaying screens\n"
           "\t\t     resolution  specify the screen resolution, the valid values are\n");
    for(int i = 0; i < DISPLAY_DEVICE_NUM; i++) {
        printf("\t\t\t %d for %s,\n", i, getDesc(ScreenResolution(i)).c_str());
    }
    printf("\t\t     x_devia     specify the x-deviation in the screen layout, 0 is default\n"
           "\t\t     3d          specify whether display 3D, not required in monocular\n"
           "\t\t     hcompress   specify whether has h-compressing in 3D, not required in "
                                 "monocular\n"
           "\t\t   NOTE, multiple screens could be specified by using '-n' consecutively.\n"
           );
}

DisplayScreen::DisplayScreen()
    : resolution(DISPLAY_DEVICE_NUM)
    , x_devia(0)
    , is_3d(false)
    , is_hcompress(false) {
}

const char* DisplayScreen::c_str() const {
    static char info[128];
    sprintf(info, "resolution:%s, x-deviation:%d, 3D:%d, hcompressL%d",
        getDesc(resolution).c_str(), x_devia, is_3d, is_hcompress);
    return info;
}

VideoViewerOption::VideoViewerOption() 
    : video_path("")
    , is_mono(false)
    , is_looped(true)
    , is_bgr(false)
    , interval(0) {
}