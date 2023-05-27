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
#include <execinfo.h>
#include <cmath>
#include <array>
#include <Eigen/Core>
#include <Eigen/Geometry>

using namespace cv;
using namespace std;

bool b_continue_session;
std::vector<WebSocketClientUtil::ImuData> imudataLeft;
std::vector<WebSocketClientUtil::ImuData> imudataRight;

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

void processImageStream(WebSocketClientUtil& socketClient, ORB_SLAM3::System& slam, std::vector<WebSocketClientUtil::ImuData>& imuData, string side, WebSocketServerUtil& server){
    while (b_continue_session)
    {
        cv::Mat img;
        while(socketClient.readImuData(imuData) == 0) {}
        while (socketClient.readImg(img) == 0) {}        
        
        uint32_t timestamp = imuData[imuData.size()-1].timestamp;
        Sophus::SE3f pose = slam.TrackMonocular(img, (double)timestamp);
        
        // Here a convertion to heading on shoe needs to happen
        double AngleLX = pose.angleX();
        double AngleLY = pose.angleY();
        double AngleLZ = pose.angleZ();
     
        cout << side << ", Timestamp: " << timestamp << ", angle X: "<< AngleLX/M_PI*180.0 << ", Angle Y: "<<  AngleLY/M_PI*180.0 <<  ", Angle Z: "<< AngleLZ/M_PI*180.0 << ", Map Id: " << slam.mpAtlas->GetCurrentMap()->GetId() << ", valid" << slam.mpAtlas->validMap << endl;

        server.send(side + ";" + std::to_string(timestamp)+ ";" +std::to_string(AngleLZ) + ";" + std::to_string(slam.mpAtlas->validMap));
    }
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
    cout << "Connecting to clients" << endl;
    WebSocketClientUtil webSocketClientLeft("192.168.4.11", "8000");
    cout << "Client 1 connected" << endl;
    WebSocketClientUtil webSocketClientRight("192.168.4.12", "8000");
    cout << "Client 2 connected" << endl;
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

    std::thread leftImageStreamProcessorThread(processImageStream, std::ref(webSocketClientLeft), std::ref(SlamL), std::ref(imudataLeft), "Left", std::ref(ws_server));
    std::thread rightImageStreamProcessorThread(processImageStream, std::ref(webSocketClientRight), std::ref(SlamR), std::ref(imudataRight), "Right", std::ref(ws_server));

    leftImageStreamProcessorThread.join();
    rightImageStreamProcessorThread.join();

    // Stop all threads
    SlamL.Shutdown();
    SlamR.Shutdown();
    t.join();
    return 0;

}