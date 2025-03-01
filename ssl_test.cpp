#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using namespace boost::asio;
using namespace boost::asio::ip;

int main()
{
    try
    {
        // Initialize IO context and SSL context
        io_context io_context;
        ssl::context ctx(ssl::context::sslv23);
        
        // Load the self-signed certificate and private key
        ctx.use_certificate_chain_file("ssl_certification/certificate.crt");
        ctx.use_private_key_file("ssl_certification/private.key", ssl::context::pem);

        // Create an SSL socket
        ssl::stream<tcp::socket> ssl_socket(io_context, ctx);

        // Resolve the server address and port
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("localhost", "4433"); // Replace with your server's address and port

        // Establish a connection
        connect(ssl_socket.lowest_layer(), endpoints);

        // Perform the SSL handshake
        ssl_socket.handshake(ssl::stream_base::client);

        // Send a "Hello World" message
        const std::string msg = "Hello World\n";
        boost::asio::write(ssl_socket, buffer(msg));

        // Read the response from the server
        char reply[128];
        size_t reply_length = boost::asio::read(ssl_socket, buffer(reply, msg.size()));
        std::cout << "Reply: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
// The code above establishes an SSL connection to a server running on localhost:4433, sends a "Hello World" message, and reads the response. Make sure to replace the server address and port with your own values. You'll also need to provide the path to your self-signed certificate and private key files. The server should be set up to accept SSL connections and respond to the "Hello World" message. You can adapt this code to fit your specific requirements and integrate it into your client application.

// g++ -o ssl_test ssl_test.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lssl -lcrypto -lws2_32 -lmswsock -pthread

/*
VSCODE gave this to lookup:

2025-02-24 16:27:07.490 [info] file:///c%3A/Coding/C%2B%2B/boost_hello_world/ssl_test.cpp Similar code at  [Ln 41, Col 35] ly_length);  std::cout << "\n";  }  catch (std::exception &e)  {  std::c...
2025-02-24 16:27:07.491 [info] License: unknown, URL: https://github.com/atulagrwl/InstantMirror/blob/2711c313b580e0e4f8246437eec8cf2eb61f521d/asio/src/examples/echo/blocking_udp_echo_client.cpp
2025-02-24 16:27:07.491 [info] License: unknown, URL: https://github.com/theSundayProgrammer/rocksdb-service/blob/f837c875974c377073d39630b823180de07c5378/src/testget.cc


*/