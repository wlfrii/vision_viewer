#include <cstdio>
#include <string>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "../src/vision_viewer.h"

bool parseCamId(std::string argstr, std::vector<int>& cam_id);

int main(int argc, char* argv[]) 
{
    printf("======================== CameraViewer Instruction ====+==================\n"
           "Command line usage:\n"
           "\t camera_viewer [\"Cameras ID\"] [optional_args]\n"
           " Explaination:\n"
           "\t\t [\"Cameras ID\"] could be one ID or two ID seperated in space.\n"
           "    Note: The current app only supports monocular or binocular. \n"
           "          When there are more than 2 cameras, the camera index specified "
           "later will be ignored.\n"
           "  optional_args: \n"
           "\t\t -w [width]\tSpecified the image width, default 1920.\n"
           "\t\t -h [width]\tSpecified the image height, default 1080.\n"
           );
    printScreenArgDesc();
    printf("-------------------------------------------------------------------------\n");
    printf("                         CameraViewer Startup \n");
    printf("-------------------------------------------------------------------------\n");

    if(argc < 2) {
        printf("No camera index is specified, camera_viewer exit.\n");
        return -1;
    }

    std::vector<int> cam_id;
    if(!parseCamId(argv[1], cam_id)) {
        std::ostringstream err;
        err << "VideoViewer: invalid camera index is given: " << std::endl;
        throw std::invalid_argument(err.str());
    }

    CameraViewerOption option;
    if((int)cam_id.size() == 1) {
        option.is_mono = true;
        option.index[0] = cam_id[0];
        printf("CameraViewer: specify monocular, cam_id: %d.\n", cam_id[0]);
    }
    else {
        option.is_mono = false;
        option.index[0] = cam_id[0];
        option.index[1] = cam_id[1];
        printf("CameraViewer: specify binocular, cam_id1: %d, cam_id2: %d.\n", 
            cam_id[0], cam_id[1]);
    }
    option.imwidth = 1920;
    option.imheight = 1080;

    int opt;
    std::string optstring = "w:h:n:";
    while((opt = getopt(argc, argv, optstring.c_str())) != -1) {
        switch (opt)
        {
        case 'w':
            option.imwidth = std::stoi(optarg);
            printf("CameraViewer: specify image width to %d.\n", option.imwidth);
            break;   
        case 'h':
            option.imheight = std::stoi(optarg);
            printf("CameraViewer: specify image height to %d.\n", option.imheight);
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

bool parseCamId(std::string argstr, std::vector<int>& cam_id) {
    cam_id.clear();

    for(auto& ch : argstr) {
        if(ch == ',') {
            ch = ' ';
        }
    }
    
    std::stringstream ss(argstr);
    int value = -1, count = 1;
    while(ss >> value) {
        cam_id.push_back(value);
    }
    
    return cam_id.size() > 0;
}
