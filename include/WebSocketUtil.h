#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

class WebSocketUtil {
public:
    WebSocketUtil(unsigned short port);
    void run();
    int send(const std::string& message);

private:
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::unique_ptr<websocket::stream<tcp::socket>> ws_;
};

#endif