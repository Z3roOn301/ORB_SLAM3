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
#include <WebSocketServerUtil.h>
#include <thread>
#include<execinfo.h>

using namespace cv;
using namespace std;

bool b_continue_session;
cv::Mat img;
std::vector<WebSocketClientUtil::ImuData> imudata;

void exit_loop_handler(int s){
   cout << "Finishing session" << endl;
   b_continue_session = false;
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char **argv)
{
    signal(SIGSEGV, handler);   // install our handler
    if(argc < 3 || argc > 5)
    {
        cerr << endl << "Usage: ./stereo_realsense_t265 path_to_vocabulary path_to_settings (trajectory_file_name)" << endl;
        return 1;
    }

    string file_name;
    string file_name2;
    bool bFileName = false;

    if (argc == 4)
    {
        file_name = string(argv[argc-1]);
        bFileName = true;
    }
    
    if (argc == 5)
    {
        file_name = string(argv[argc-2]);
        file_name2 = string(argv[argc-1]);
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
    WebSocketServerUtil ws_server(8080);
    std::thread t(&WebSocketServerUtil::run, &ws_server);
    namedWindow("Display window");

    cout << endl << "-------" << endl;
    cout.precision(17);

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SlamL(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , true, 0, file_name);
    ORB_SLAM3::System SlamR(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , false, 0, file_name2);

    double t_resize = 0.f;
    double t_track = 0.f;

    uint32_t timestampL = 0;
    uint32_t timestampR = 0;

    while (b_continue_session)
        {
        while (ws.readImuData(imudata) == 0) {}
        while (ws.readImg(img) == 0) {}
        timestampL = imudata[imudata.size()-1].timestamp;
        timestampR = imudata[imudata.size()-1].timestamp; //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RSO: This is a bug, timestampR should be the timestamp of the right camera
        Sophus::SE3f PoseL = SlamL.TrackMonocular(img, (double)timestampL);
        Sophus::SE3f PoseR = SlamR.TrackMonocular(img, (double)timestampR);
        cout << "Timestamp: " <<timestampL << "    Angle X: "<< PoseL.angleX() << "   Angle Y: "<<  PoseL.angleY() <<  "   Angle Z: "<< PoseL.angleZ() <<  "    Map ID L: " << SlamL.mpAtlas->GetCurrentMap()->GetId() <<  "    Map ID R: " << SlamR.mpAtlas->GetCurrentMap()->GetId()<< endl << " L valid" << SlamL.mpAtlas->validMap << " R valid" << SlamR.mpAtlas->validMap << endl;

        ws_server.send(std::to_string(timestampL)+ ";" + std::to_string(PoseL.angleX())+ ";"  + std::to_string(PoseL.angleY())+  ";" +std::to_string(PoseL.angleZ()) + ";" + std::to_string(SlamL.mpAtlas->validMap) + ";" +
        std::to_string(timestampR)+ ";" +  std::to_string(PoseR.angleX()) +  ";" +std::to_string(PoseR.angleY())+ ";" + std::to_string(PoseR.angleZ()) + ";" + std::to_string(SlamR.mpAtlas->validMap));
    }

// Stop all threads
SlamL.Shutdown();
SlamR.Shutdown();
t.join();
return 0;

}