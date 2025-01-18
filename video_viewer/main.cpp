#include <cstdio>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include "../src/vision_viewer.h"

bool parseScreenInfo(std::string argstr, std::vector<DisplayScreen>& screens);

int main(int argc, char* argv[]) 
{
    printf("========================= VideoViewer Instruction ========================\n"
           "Command line usage:\n"
           "\t video_viewer [bino_video_path] [optional_args]\n"
           "  optional_args: \n"
           "\t\t -m\tSpecify the given video is monocular video (defaultly)\n"
           "\t\t -l\tSpecify the given video will be looped display (defaultly)\n"
           "\t\t -b\tSpecify the given video is BGR format (RGB is default)\n"
           "\t\t -t [value]\tSpecify the image refresh interval is [value] ms\n"
           );
    printScreenArgDesc();
    printf("-------------------------------------------------------------------------\n");
    printf("                           VideoViewer Startup \n");
    printf("-------------------------------------------------------------------------\n");

    if(argc < 2) {
        printf("No video_path is specified, video_viewer exit.\n");
        return -1;
    }

    VideoViewerOption option;
    option.video_path = argv[1];
    option.screens.clear();

    int opt;
    std::string optstring = "mlbt:n:";
    while((opt = getopt(argc, argv, optstring.c_str())) != -1) {
        switch (opt)
        {
        case 'm':
            option.is_mono = true;
            printf("VideoViewer: the input video is specified as monocular video.\n");
            break;
        case 'l':
            option.is_looped = true;
            printf("VideoViewer: the input video is specified to be looped display.\n");
            break;
        case 'b':
            option.is_bgr = true;
            printf("VideoViewer: the input video is specified as BGR format.\n");
            break;   
        case 't':
            option.interval = std::stoi(optarg);
            printf("VideoViewer: the input video will be refreshed in %d ms interval.\n",
                    option.interval);
            break;  
        case 'n':
            if(!parseScreenInfo(optarg, option.screens)) {
                std::ostringstream err;
                err << "VideoViewer: invalid screen info is given: " << std::endl;
                throw std::invalid_argument(err.str());
            }
            break;
        default:
            break;
        }
    }
    
    VisionViewer video_viewer(option);
    video_viewer.startShow();

    return 0;
}

