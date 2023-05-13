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
#include <WebSocketClientUtil.h>

using namespace cv;
using namespace std;

bool b_continue_session;

//variable which saves the image
cv::Mat img;
//variable which saves the imudata
std::vector<WebSocketClientUtil::ImuData> imudata;

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
    WebSocketClientUtil ws("192.168.4.11", "8000");

    namedWindow("Display window");

    cout << endl << "-------" << endl;
    cout.precision(17);

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , true, 0, file_name);
    float imageScale = SLAM.GetImageScale();
    // cout << "imageScale: " << imageScale << endl;

    double t_resize = 0.f;
    double t_track = 0.f;

    double timestamp = 0;

    vector<ORB_SLAM3::IMU::Point> vImuMeas;

while (b_continue_session)
    {
    while (ws.readImuData(imudata) == 0) {}
    while (ws.readImg(img) == 0) {}

    for(int i=0; i<imudata.size(); ++i)
    {
        ORB_SLAM3::IMU::Point lastPoint(imudata[i].ax, imudata[i].ay, imudata[i].az, 
                                imudata[i].gx, imudata[i].gy, imudata[i].gz, imudata[i].timestamp*1000);
        vImuMeas.push_back(lastPoint);
    }
    
    timestamp = double(imudata[imudata.size()-1].timestamp*1000);

    Sophus::SE3f Pose = SLAM.TrackMonocular(img, timestamp, vImuMeas);
    cout << "   Timestamp: " <<timestamp << "    Angle X: "<< Pose.angleX() << "   Angle Y: "<<  Pose.angleY() <<  "   Angle Z: "<< Pose.angleZ() << endl;

    vImuMeas.clear();
}

// Stop all threads
SLAM.Shutdown();

return 0;

}