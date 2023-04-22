// #include <iostream>
// #include <boost/asio.hpp>
// #include <boost/beast.hpp>

// using tcp = boost::asio::ip::tcp;
// namespace websocket = boost::beast::websocket;

// template<class NextLayer>
// void setup_stream(websocket::stream<NextLayer>& ws)
// {
//     // These values are tuned for Autobahn|Testsuite, and
//     // should also be generally helpful for increased performance.
//     websocket::permessage_deflate pmd;
//     pmd.client_enable = true;
//     pmd.server_enable = true;
//     pmd.compLevel = 3;
//     ws.set_option(pmd);
//     ws.auto_fragment(false);

//     // Autobahn|Testsuite needs this
//     ws.read_message_max(64 * 1024 * 1024);
// }

// int main()
// {
//     boost::asio::io_context io_context;
//     tcp::endpoint endpoint{tcp::v4(), 8080};
//     tcp::acceptor acceptor{io_context, endpoint};
//     boost::beast::error_code ec;

//     while (true) {
//         std::cout << "Starting..." << std::endl;
//         tcp::socket socket{io_context};
//         acceptor.accept(socket);
//         std::cout << "Started" << std::endl;

//         try {
//             websocket::stream<tcp::socket> ws{std::move(socket)};
//             // setup_stream(ws);
//             ws.accept();
//             std::cout << "accepted" << std::endl;

//             while (true) {
//                 boost::beast::flat_buffer buffer;
//                 boost::beast::flat_buffer b;
//                 boost::beast::ostream(b) << "Hello, world!\n";
//                 ws.write(b.data(), ec);
//                 sleep(1);
//                 // ws.read(buffer, ec);
//                 if(ec == websocket::error::closed)
//                     break;
//                 if(ec){
//                     std::cout << "Error " << ec << std::endl;
//                     break;
//                 }
//                 // if(!ec){
//                 //     std::cout << "read stuff" << std::endl;
//                 // ws.text(ws.got_text());
//                 // ws.write(buffer.data(), ec);
//                 // }
//             }
//         }
//         catch (const std::exception& ex) {
//             std::cerr << "Exception: " << ex.what() << std::endl;
//         }
//     }

//     return 0;
// }
#include <iostream>
#include <thread>
#include "WebSocketUtil.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
int main()
{
    WebSocketUtil server(8080);
    std::thread server_thread(&WebSocketUtil::run, &server);

    while (1)
    {
        server.send("Hello, world!\n");
        sleep(1);
    }
    
    server_thread.join();
    return 0;
}