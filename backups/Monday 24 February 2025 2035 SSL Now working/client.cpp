#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>

using boost::asio::ip::tcp;

void read_from_server(boost::asio::ssl::stream<tcp::socket>& ssl_socket)
{
    try
    {
        for (;;)
        {
            char reply[1024];
            boost::system::error_code error;
            size_t reply_length = ssl_socket.read_some(boost::asio::buffer(reply), error);

            if (error == boost::asio::error::eof)
            {
                // Connection closed cleanly by peer
                std::cout << "Connection closed by server. Code will stop running now.\n";
                break;
            }
            else if (error)
            {
                throw boost::system::system_error(error); // Some other error
            }

            std::cout << "Received message: ";
            std::cout.write(reply, reply_length);
            std::cout << "\n";
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

void client(boost::asio::io_context& io_context, boost::asio::ssl::context& ssl_context, const std::string& host, short port)
{
    try
    {
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

        boost::asio::ssl::stream<tcp::socket> ssl_socket(io_context, ssl_context);
        boost::asio::connect(ssl_socket.lowest_layer(), endpoints);

        std::cout << "Connected to server at " << host << ":" << port << std::endl;

        // Perform the SSL handshake
        ssl_socket.handshake(boost::asio::ssl::stream_base::client);

        std::thread(read_from_server, std::ref(ssl_socket)).detach();

        std::string message;
        while (std::getline(std::cin, message))
        {
            boost::asio::write(ssl_socket, boost::asio::buffer(message));
            std::cout << "Sent message: " << message << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: client <host> <port>\n";
        return 1;
    }

    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);

    ssl_context.load_verify_file("ssl_certification/certificate.crt");

    client(io_context, ssl_context, argv[1], std::atoi(argv[2]));

    return 0;
}


// Build: g++ more_advanced_boost.cpp -o more_advanced_boost -lboost_system -pthread
// Run: ./more_advanced_boost 1234
// g++ -o boost_hello_world boost_hello_world.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock 
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread

// VSCODE prompted this as well:
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread -std=c++11
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread -std=c++11