#include "WebSocketClientUtil.h"
#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat img;

int main() {
    WebSocketClientUtil ws("192.168.4.11", "8000");

    while (true) {
        try {
            if (ws.readImg(img) == 1) {
                cv::imshow("Image", img);
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
    return 0;
}