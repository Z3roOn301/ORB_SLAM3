#include "WebSocketClientUtil.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
    std::ofstream imudatafile("IMUdata.csv");
    imudatafile << "#timestamp [ns],w_RS_S_x [rad s^-1],w_RS_S_y [rad s^-1],w_RS_S_z [rad s^-1],a_RS_S_x [m s^-2],a_RS_S_y [m s^-2],a_RS_S_z [m s^-2]\n";
    std::ofstream imgdatafile("IMGdata.csv");
    imgdatafile << "#timestamp [ns],filename\n";
    long long lasttimestamp = 0;

    WebSocketClientUtil ws("192.168.4.11", "8000");
    // std::thread client_thread(&WebSocketClientUtil::run, &ws);

    while (true) {
        try {
            std::string message = ws.read();
            if (message.empty()) {
                continue;
            }

            std::vector<uchar> data(message.begin(), message.end());

            if (data.size() > 2000) {
                cv::Mat img = cv::imdecode(data, cv::IMREAD_GRAYSCALE);
                cv::imshow("Stream", img);
                cv::imwrite("imucapture/" + std::to_string(lasttimestamp) + ".jpg", img);
            } else {
                for (int i = 0; i < data.size(); i += 32) {
                    long long timestamp;
                    float ax, ay, az, gx, gy, gz;
                    memcpy(&timestamp, &data[i], 8);
                    memcpy(&ax, &data[i + 8], 4);
                    memcpy(&ay, &data[i + 12], 4);
                    memcpy(&az, &data[i + 16], 4);
                    memcpy(&gx, &data[i + 20], 4);
                    memcpy(&gy, &data[i + 24], 4);
                    memcpy(&gz, &data[i + 28], 4);

                    std::cout << timestamp << " " << ax << " " << ay << " " << az << " " << gx << " " << gy << " " << gz << std::endl;
                    lasttimestamp = timestamp * 1000;
                    imudatafile << lasttimestamp << "," << gx << "," << gy << "," << gz << "," << ax << "," << ay << "," << az << "\n";
                }
                imgdatafile << lasttimestamp << "," << lasttimestamp << ".jpg\n";
            } 
            // Check if the 'q' key was pressed
            if (cv::waitKey(1) == 'q') {
                break;
            }
        }
        catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    // client_thread.join();
    return 0;
}