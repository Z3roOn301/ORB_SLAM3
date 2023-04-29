#ifndef WEBSOCKETCLIENTUTIL_H
#define WEBSOCKETCLIENTUTIL_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>
#include <string>
#include <opencv2/opencv.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class WebSocketClientUtil {
public:
    WebSocketClientUtil(const std::string& host, const std::string& port);
    ~WebSocketClientUtil();
    void run();
    int send(const std::string& message);
    std::string read();
    #pragma pack(4)
    struct ImuData {
        uint32_t timestamp;
        float heading;
        float ax, ay, az, gx, gy, gz;
    };
    #pragma pack(0)
    // ImuData readImuData();
    int readImuData(std::vector<ImuData>& imuData);
    int readImg(cv::Mat& img);

private:
    net::io_context io_context_;
    tcp::resolver resolver_;
    std::unique_ptr<websocket::stream<tcp::socket>> ws_;
    std::string host_;
    std::string port_;
};

#endif