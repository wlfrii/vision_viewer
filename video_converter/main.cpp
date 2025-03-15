#include <cstdio>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) 
{
    printf("========================= VideoConverter Instruction ========================\n"
           "Command line usage:\n"
           "\t video_converter [video_path] [optional_args]\n"
           "  optional_args: \n"
           "\t\t -b\tSpecify the given video is BGR format (RGB is default)\n"
           "\t\t -w\tSpecify the output width of video.\n"
           "\t\t -h\tSpecify the output width of video.\n"
           );
    printf("-------------------------------------------------------------------------\n");
    printf("                           VideoViewer Startup \n");
    printf("-------------------------------------------------------------------------\n");

    if(argc < 2) {
        printf("VideoConverter: No video_path is specified, exit.\n");
        return -1;
    }
    std::string video_path = argv[1];

    cv::VideoCapture cap(video_path);
    if(!cap.isOpened()) {
        std::ostringstream err;
        err << "VideoConverter: unable to open input video file: " 
            << video_path << std::endl;
        throw std::invalid_argument(err.str());
    }

    int imwidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int imheight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    printf("VideoConverter: Input video property: %d x %d resolution, with %f FPS\n", 
            imwidth, imheight, fps);    

    bool is_bgr = false;
    int opt;
    std::string optstring = "bw:h:";
    while((opt = getopt(argc, argv, optstring.c_str())) != -1) {
        switch (opt)
        {
        case 'b':
            is_bgr = true;
            printf("VideoConverter: the input video is specified as BGR format.\n");
            break; 
        case 'w':
            imwidth = std::stoi(optarg);
            printf("VideoConverter: the output video width is set to %d.\n", imwidth);
            break;
        case 'h':
            imheight = std::stoi(optarg);
            printf("VideoConverter: the output video height is set to %d.\n", imheight);
            break;
        default:
            break;
        }
    }

    cv::Size out_size = cv::Size(imwidth, imheight);
    cv::VideoWriter video_writer(video_path+".avi",  
                                 cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 
                                 fps, out_size, true);
    if (!video_writer.isOpened()) {
        std::cout << "VideoConverter: cannot open the video writer, exit!\n";
        return false;
    }

    size_t loop_count = 0;
    uint8_t idx = 0;
    cv::Mat frame;
    while(cap.read(frame)) {
        // Convert color if needed.
        if(is_bgr) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        }

        cv::resize(frame, frame, out_size);
        video_writer.write(frame);

        cv::imshow("VideoConverter", frame);
        cv::waitKey(10);
    }
    video_writer.release();

    printf("VideoConverter: video is convertered done.\n");

    return 0;
}

