#include <cstdio>
#include <string>
#include "./src/endo_viewer.h"

int main(int argc, char* argv[]) 
{
    printf("================ Endoscope viewer startup ================\n"
           "Command line usage:\n"
           "\t endo_viewer [left_cam_id (0 for default)] [right_cam_id (1 for default)]\n");

    if(argc == 2) {
        printf("ERROR: Please specified another cam index.\n");
        return -1;
    }

    uint8_t left_cam_id = 0;
    uint8_t right_cam_id = 1;
    bool is_write_video = false;
    if(argc == 3) {
        left_cam_id = std::stoi(argv[1]);
        right_cam_id = std::stoi(argv[2]);
    }
    if(argc == 4) {
        left_cam_id = std::stoi(argv[1]);
        right_cam_id = std::stoi(argv[2]);
        is_write_video = std::stoi(argv[3]);
    }

    EndoViewer endo_viewer;
    endo_viewer.startup(left_cam_id, right_cam_id, is_write_video);

    return 0;
}


