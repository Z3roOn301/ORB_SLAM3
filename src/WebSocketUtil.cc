#include "WebSocketUtil.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

// Constructor
WebSocketUtil::WebSocketUtil(unsigned short port)
    : io_context_{}, acceptor_{io_context_, tcp::endpoint{tcp::v4(), port}}
{}

// Run the server
void WebSocketUtil::run() {
    // Accept incoming connections
    while (true) {
        try {

            std::cout << "Starting..." << std::endl;
            tcp::socket socket{io_context_};
            acceptor_.accept(socket);
            std::cout << "Started" << std::endl;

            // Construct a websocket stream around the socket
            ws_ = std::make_unique<websocket::stream<tcp::socket>>(std::move(socket));
            // Perform the websocket handshake
            ws_->accept();
            std::cout << "accepted" << std::endl;

            // Check if the connection is still active
            while (true) {
                sleep(1);
                if(!ws_->is_open()) {
                    break;
                }
                    
            }
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception: " << ex.what() << std::endl;
        }
    }
}


// Send a message to the client
int WebSocketUtil::send(const std::string& message) {
    if (!ws_ || !ws_->is_open()) {
        return 0;
    }

    boost::beast::flat_buffer buffer;
    boost::beast::ostream(buffer) << message;
    ws_->write(buffer.data());

    return 1;
}