#include <iostream>
#include <string>
#include "shm_buf.h"
#include <unistd.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp> 
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// Suppose the video is 3-channel 1080p 
static const uint32_t WIDTH = 1920;
static const uint32_t HEIGHT = 1080;
static const uint32_t CHANNELS = 3;

void producer()
{
    const char *shm_name = "shm_name_test";
    SharedMemoryBuffer::remove_shm(shm_name);
    SharedMemoryBuffer shmbuf(shm_name, WIDTH*HEIGHT*CHANNELS*100 + 12);

    int64 t0 = cv::getTickCount();;
    int64 t1 = 0;
    string fps;
    cv::Mat frame;
    int nFrames = 0;
    cout << "Opening video..." << endl;

    VideoCapture cap("rtsp://192.21.1.235:554/LiveMedia/ch1/Media1");
    while (cap.isOpened()) 
    {
        cap >> frame;
        if (frame.empty())
        {
            std::cerr << "ERROR: Can't grab video frame." << endl;
            break;
        }

        nFrames++;

        if (!frame.empty()) 
        {
            if (nFrames % 10 == 0)
            {
                const int N = 10;
                int64 t1 = cv::getTickCount();
                fps = " Send FPS:" + to_string((double)getTickFrequency() * N / (t1 - t0)) + "fps";    
                t0 = t1;
            }
            cv::putText(frame, fps, Point(100, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(255, 255, 255),1);
        }
        auto fsize = frame.cols * frame.rows * frame.channels();
        shmbuf.write_shm(frame.data, fsize);
        // std::cout << "fsize: " << fsize << std::endl;
        
        if ((waitKey(1) & 0xFF) == 'q')
            break;
    }
}

int main(int argc, char *argv[])
{
    producer();
    return 0;
}
