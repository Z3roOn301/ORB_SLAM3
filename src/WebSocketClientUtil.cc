#include "WebSocketClientUtil.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Constructor
WebSocketClientUtil::WebSocketClientUtil(const std::string& host, const std::string& port)
    : io_context_{}, resolver_{io_context_}, ws_{std::make_unique<websocket::stream<tcp::socket>>(io_context_)}, host_{host}, port_{port}
{
    auto const results = resolver_.resolve(host_, port_);
    net::connect(ws_->next_layer(), results.begin(), results.end());
    ws_->handshake(host_, "/stream");
}

// Destructor
WebSocketClientUtil::~WebSocketClientUtil() {
    if (ws_ && ws_->is_open()) {
        ws_->close(websocket::close_code::normal);
    }
}

// Run the client
void WebSocketClientUtil::run() {
    // Check if the connection is still active
    while (true) {
        try {
            sleep(1);
            if(!ws_->is_open()) {
                auto const results = resolver_.resolve(host_, port_);
                net::connect(ws_->next_layer(), results.begin(), results.end());
                ws_->handshake(host_, "/");
            }
        } catch (const std::exception& ex) {
            std::cerr << "Exception in run: " << ex.what() << std::endl;
        }
    }
}

// Send a message to the server
int WebSocketClientUtil::send(const std::string& message) {
    if (!ws_ || !ws_->is_open()) {
        return 0;
    }

    boost::beast::flat_buffer buffer;
    boost::beast::ostream(buffer) << message;
    ws_->write(buffer.data());

    return 1;
}

// Read a message from the server
std::string WebSocketClientUtil::read() {
    if (!ws_ || !ws_->is_open()) {
        return "";
    }

    boost::beast::flat_buffer buffer;
    ws_->read(buffer);
    return boost::beast::buffers_to_string(buffer.data());
}