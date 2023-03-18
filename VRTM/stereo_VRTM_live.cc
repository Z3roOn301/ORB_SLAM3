#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <opencv2/core/core.hpp>
#include <System.h>
#include <opencv2/opencv.hpp>

using namespace cv;

using namespace std;



bool b_continue_session;

void exit_loop_handler(int s){
   cout << "Finishing session" << endl;
   b_continue_session = false;

}

int main(int argc, char **argv)
{

    if(argc < 3 || argc > 4)
    {
        cerr << endl << "Usage: ./stereo_realsense_t265 path_to_vocabulary path_to_settings (trajectory_file_name)" << endl;
        return 1;
    }

    string file_name;
    bool bFileName = false;

    if (argc == 4)
    {
        file_name = string(argv[argc-1]);
        bFileName = true;
    }

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = exit_loop_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
    b_continue_session = true;


    //ESP32 URL
    std::string URL_R = "http://192.168.4.11";
    std::string URL_L = "http://192.168.4.12";

    Mat image_L;
    Mat image_R;
    Mat imageEq_L;
    Mat imageEq_R;

    namedWindow("Display window");

    VideoCapture cap_L(URL_L + ":80/mjpeg/1");
    VideoCapture cap_R(URL_R + ":80/mjpeg/1");

    if (!cap_L.isOpened() || !cap_R.isOpened()) {
    cout << "cannot open camera";
    }

    cout << endl << "-------" << endl;
    cout.precision(17);

    /*cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl;
    cout << "IMU data in the sequence: " << nImu << endl << endl;*/

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1],argv[2],ORB_SLAM3::System::STEREO, true, 0, file_name);
    float imageScale = SLAM.GetImageScale();

    double t_resize = 0.f;
    double t_track = 0.f;

    int timestamp = 0;

while (b_continue_session)
    {
    cap_L >> image_L;
    cap_R >> image_R;

    // equalizeHist(image_L, imageEq_L);
    // equalizeHist(image_R, imageEq_R);
    // image_L = imageEq_L;
    // image_R = imageEq_R;

    // imshow("Display window L", image_L);
    // imshow("Display window R", image_R);

    // Pass the image to the SLAM system
    Sophus::SE3f Pose = SLAM.TrackStereo(image_L, image_R, (double)timestamp++);


    cout << "Angle X "<< Pose.angleX() << "   Angle Y "<<  Pose.angleY() <<  "   Angle Z "<< Pose.angleZ() << endl;

}

// Stop all threads
SLAM.Shutdown();

return 0;

}