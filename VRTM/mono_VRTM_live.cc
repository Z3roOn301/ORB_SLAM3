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

using namespace cv;
using namespace std;

bool b_continue_session;
cv::Mat img;
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
    string file_name2;
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
    WebSocketServerUtil ws_server(8080);
    std::thread t(&WebSocketServerUtil::run, &ws_server);
    namedWindow("Display window");

    cout << endl << "-------" << endl;
    cout.precision(17);

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SlamL(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , true, 0, file_name);

    double t_resize = 0.f;
    double t_track = 0.f;

    uint32_t timestamp = 0;

    while (b_continue_session)
        {
        while (ws.readImuData(imudata) == 0) {}
        while (ws.readImg(img) == 0) {}
        timestamp = imudata[imudata.size()-1].timestamp;
        Sophus::SE3f PoseL = SlamL.TrackMonocular(img, (double)timestamp);
        cout << "Timestamp: " <<timestamp << "    Angle X: "<< PoseL.angleX() << "   Angle Y: "<<  PoseL.angleY() <<  "   Angle Z: "<< PoseL.angleZ() << endl;
        
        ws_server.send(std::to_string(timestamp)+ ";" + std::to_string(PoseL.angleX())+ ";"  + std::to_string(PoseL.angleY())+  ";" +std::to_string(PoseL.angleZ()));
    }

// Stop all threads
SlamL.Shutdown();
//SlamR.Shutdown();
t.join();
return 0;

}







// #include <signal.h>
// #include <stdlib.h>
// #include <iostream>
// #include <algorithm>
// #include <fstream>
// #include <chrono>
// #include <ctime>
// #include <sstream>
// #include <opencv2/core/core.hpp>
// #include <System.h>
// #include <opencv2/opencv.hpp>
// #include <WebSocketClientUtil.h>
// #include <WebSocketServerUtil.h>
// #include <thread>

// using namespace cv;
// using namespace std;

// bool b_continue_session;
// cv::Mat img;
// std::vector<WebSocketClientUtil::ImuData> imudata;

// void exit_loop_handler(int s){
//     cout << "Finishing session" << endl;
//     b_continue_session = false;
// }

// void slam_thread(ORB_SLAM3::System* Slam, string file_name, double timestamp, cv::Mat& img) {
//     Sophus::SE3f Pose = Slam->TrackMonocular(img, timestamp);
//     cout << "Timestamp: " << timestamp << "    Angle X: "<< Pose.angleX() << "   Angle Y: "<<  Pose.angleY() <<  "   Angle Z: "<< Pose.angleZ() << endl;
// }

// int main(int argc, char **argv) {

//     // Check input arguments
//     if(argc < 3 || argc > 5) {
//         cerr << endl << "Usage: ./stereo_realsense_t265 path_to_vocabulary path_to_settings (trajectory_file_name)" << endl;
//         return 1;
//     }

//     // Parse input arguments
//     string file_name;
//     string file_name2;
//     bool bFileName = false;

//     if (argc == 4) {
//         file_name = string(argv[argc-1]);
//         bFileName = true;
//     }

//     if (argc == 5) {
//         file_name = string(argv[argc-2]);
//         file_name2 = string(argv[argc-1]);
//         bFileName = true;
//     }

//     // Set up signal handler
//     struct sigaction sigIntHandler;
//     sigIntHandler.sa_handler = exit_loop_handler;
//     sigemptyset(&sigIntHandler.sa_mask);
//     sigIntHandler.sa_flags = 0;
//     sigaction(SIGINT, &sigIntHandler, NULL);

//     //ESP32 URL
//     WebSocketClientUtil ws("192.168.4.11", "8000");
//     WebSocketServerUtil ws_server(8080);
//     std::thread t(&WebSocketServerUtil::run, &ws_server);
//     namedWindow("Display window");

//     // Initialize SLAM systems
//     ORB_SLAM3::System SlamL(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , true, 0, file_name);
//     ORB_SLAM3::System SlamR(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR , true, 0, file_name2);

//     // Initialize variables
//     b_continue_session = true;
//     double t_resize = 0.f;
//     double t_track = 0.f;
//     int timestamp = 0;
//     int timestamp = 0;

    // // Main loop
    // int map_id = 0;
    // std::unordered_set<int> map_ids;
    // while (true) {
    //     // Get the latest frame from the camera
    //     FramePtr frame = camera->get_frame();

    //     // Check if the frame is valid
    //     if (!frame) {
    //         break;
    //     }

    //     // Check if the timestamp of the current frame is older than the previous frame
    //     if (frame->timestamp < last_timestamp) {
    //         std::cerr << "ERROR: Frame with a timestamp older than previous frame detected!" << std::endl;
    //         continue;
    //     }

    //     last_timestamp = frame->timestamp;

    //     // Create a new map if the current map ID is not already in use
    //     if (map_ids.find(map_id) == map_ids.end()) {
    //         atlas->create_new_map(map_id);
    //         std::cerr << "Creation of new map with id: " << map_id << std::endl;
    //         map_ids.insert(map_id);
    //     }

    //     // Process the frame with ORB-SLAM3
    //     atlas->process(frame);

    //     // Update the current camera pose
    //     Pose PoseL = atlas->get_current_pose();

    //     // Print the camera pose
    //     std::cerr << "Timestamp: " << frame->timestamp
    //             << "    Angle X: " << PoseL.angleX()
    //             << "   Angle Y: " <<  PoseL.angleY()
    //             << "   Angle Z: " << PoseL.angleZ() << std::endl;

    //     // Save the map periodically
    //     if (frame->id % save_interval == 0) {
    //         atlas->save_map(map_id, map_dir);
    //     }
    // }


    // while (b_continue_session)
    // {
    //     while (ws.readImuData(imudata) == 0) {}
    //     while (ws.readImg(img) == 0) {}

    //     timestamp = double(imudata[imudata.size()-1].timestamp);
    //     timestamp = timestamp;

    //     // create threads for left and right ORB-SLAM systems
    //     std::thread t_L([&]{
    //         Sophus::SE3f PoseL = SlamL.TrackMonocular(img, timestamp);
    //         cout << "Timestamp: " << timestamp << "    Angle X: " << PoseL.angleX() << "   Angle Y: " <<  PoseL.angleY() <<  "   Angle Z: " << PoseL.angleZ() << endl;
    //         ws_server.send(std::to_string(timestamp) + ";" + std::to_string(PoseL.angleX()) + ";"  + std::to_string(PoseL.angleY()) + ";" + std::to_string(PoseL.angleZ()));
    //     });

    //     std::thread t_R([&]{
    //         Sophus::SE3f PoseR = SlamR.TrackMonocular(img, timestamp);
    //         ws_server.send(std::to_string(timestamp) + ";" + std::to_string(PoseR.angleX()) + ";" + std::to_string(PoseR.angleY()) + ";" + std::to_string(PoseR.angleZ()));
    //     });

    //     // wait for both threads to finish
    //     t_L.join();
    //     t_R.join();
    // }


    // // Main loop
    // while (b_continue_session)
    // {
    //     while (ws.readImuData(imudata) == 0) {}
    //     while (ws.readImg(img) == 0) {}

    //     timestamp = double(imudata[imudata.size()-1].timestamp);
    //     timestamp = timestamp;

    //     // create threads for left and right ORB-SLAM systems
    //     std::thread t_L([&]{
    //         Sophus::SE3f PoseL = SlamL.TrackMonocular(img, timestamp);
    //     });

    //     std::thread t_R([&]{
    //         Sophus::SE3f PoseR = SlamR.TrackMonocular(img, timestamp);
    //     });

    //     // wait for both threads to finish
    //     t_L.join();
    //     t_R.join();

    //     // Get poses from both systems
    //     // Sophus::SE3f PoseL = SlamL.GetCurrentPose();
    //     // Sophus::SE3f PoseR = SlamR.GetCurrentPose();

    //     // print and send pose information over websocket
    //     cout << "Timestamp: " << timestamp << "    Angle X: " << PoseL.angleX() << "   Angle Y: " <<  PoseL.angleY() <<  "   Angle Z: " << PoseL.angleZ() << endl;

    //     ws_server.send(std::to_string(timestamp) + ";" + std::to_string(PoseL.angleX()) + ";"  + std::to_string(PoseL.angleY()) + ";" + std::to_string(PoseL.angleZ()) + ";" +
    //     std::to_string(timestamp) + ";" + std::to_string(PoseR.angleX()) + ";" + std::to_string(PoseR.angleY()) + ";" + std::to_string(PoseR.angleZ()));
    // }

//     return 0;
// }