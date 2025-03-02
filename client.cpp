#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>

// Overview:
// The io_context object is used to manage the I/O services. It's the main big boss that runs the show 😎
// The server has its own io_context object. Each client has its own io_context object.
// The io_context object is created in the main function.
// Boost ASIO means we can have many operations running all at the same time - sending and receiving simultaneously.

// Same as for server.cpp
using boost::asio::ip::tcp;

// This function reads data from the server AND outputs to the screen.
// It reads data from the SSL/TLS-encrypted socket and stores it in a buffer for processing.
void read_from_server(boost::asio::ssl::stream<tcp::socket> &ssl_socket)
{
    try
    {
        // Loop runs indefinitely until the server closes the connection
        for (;;)
        {
            // Declare a char array to store the reply from the server
            // This is our buffer that we use to store the data that is read from the server.
            char reply[1024];

            // Make an error code object to store any errors that occur
            boost::system::error_code error;

            // Read some data from the server and store it in the reply buffer
            // The read_some function reads data from the socket and stores it in the buffer.
            // The function returns the number of bytes read. We need the size of the buffer to know how many bytes ti write further below.
            size_t reply_length = ssl_socket.read_some(boost::asio::buffer(reply), error);

            // If there is an error, we check if the error is an EOF error
            // If it is, we print a message and break out of the loop
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

            // Added colouring to the text.
            // You need to set the colour back to normal after you've finished with the colouring.
            // We create colour objects to set and reset the colour of the text.
            std::string colour = "\033[38;5;112m";
            std::string reset = "\033[0m";
            // orange = "\033[38;2;255;165;0m" olive_green = "\033[38;5;112m" light_blue = "\033[38;5;153m" light_purple = "\033[38;5;189m" light_green = "\033[38;5;120m" light_red = "\033[38;5;196m" light_yellow = "\033[38;5;226m" light_orange = "\033[38;5;215m" light_pink = "\033[38;5;213m" light_cyan = "\033[38;5;87m" light_brown = "\033[38;5;130m" light_grey = "\033[38;5;250m" light_black = "\033[38;5;232m" light_white = "\033[38;5;231m" gray = "\033[1;30m" yellow = "\033[1;33m"

            // We print the data that was received from the server.
            // TODO - add the name of the client to the message.
            // To print in colour, we have to use in the format given below. I'll change this when I do the GUI version.
            // std::cout << colour << "Received message: ";
            std::cout << colour;
            std::cout.write(reply, reply_length);
            std::cout << reset << "\n";
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

// This function is the client function that connects to the server
// It takes the io_context object, the ssl_context object, the host name, and the port number as arguments.
// I didn't make a class for the client because it's a simple client that connects to the server and sends messages.
// void client(boost::asio::io_context &io_context, boost::asio::ssl::context &ssl_context, const std::string &host, short port)
void client(boost::asio::io_context &io_context, boost::asio::ssl::context &ssl_context, const std::string &host, short port, const std::string &name) // I need to send the name of the chat client, this is a bit of a hack I think, I would prefer to pass in as an object
{
    // std::cout << "Client function has been called." << std::endl; // Used for debugging

    try
    {
        // The tcp::resolver class is used to resolve hostnames into IP addresses and service names into port numbers.
        // It can take something like www.domain.com and convert it into an IP address, along with translating a service name like "http" into a port number like 80.
        // The resolver object is created with the io_context object, which is used to perform asynchronous operations.
        // The resolver is used to perform the resolution process using the address and service names provided in string format.
        tcp::resolver resolver(io_context);
        // In our case, if we give "localhost" as the hostname, the code will resolve it to the IP address 127.0.0.1.

        // The resolve function is called with the hostname and port number as arguments.
        // The resolve function returns a list of endpoints that can be used to connect to the server.
        // endpoints is a list of IP addresses and port numbers that can be used to connect to the server.
        // host is for example "localhost" and port is for example 1234.
        tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

        // The ssl_socket object is created with the io_context object and the ssl_context object.
        // Once we have the SSL Socket object, we can connect to the server and send messages.
        // The ssl::stream class is used to apply SSL/TLS to a normal TCP socket that would otherwise be encrypted.
        boost::asio::ssl::stream<tcp::socket> ssl_socket(io_context, ssl_context);

        // Here we connect to the server using the endpoints that we resolved earlier.
        // The lowest_layer function is used to get the underlying socket object.
        // We now have access to the raw TCP socket wrapped by the SSL/TLS layer.
        // boost::asio::connect(ssl_socket.lowest_layer(), endpoints);
        // std::cout << "Connected to server at " << host << ":" << port << std::endl;

        try
        {
            boost::asio::connect(ssl_socket.lowest_layer(), endpoints);
            std::cout << "Connected to server at " << host << ":" << port << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }

        // Perform the SSL handshake
        // ssl_socket.handshake(boost::asio::ssl::stream_base::client);
        // std::cout << "Client side: SSL handshake completed successfully with the server.\n";
        try
        {
            ssl_socket.handshake(boost::asio::ssl::stream_base::client);
            std::cout << "Client side: SSL handshake completed successfully with the server." << std::endl
                      << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception during SSL handshake: " << e.what() << std::endl;
        }

        
        // ---------------------------------- //
        // Added Sunday 02 March 2025 0009 - I need to send the name of the chat client, this is a bit of a hack I think, I would prefer to pass in as an object
        // We write the name to the server using the ssl_socket object. This is the first message that the server receives from the client. It reads in and stores the name of the client in a newly added variable. 
        boost::asio::write(ssl_socket, boost::asio::buffer(name));
        std::cout << "Name sent to server." << std::endl;
        std::cout << std::endl << std::endl;
        // ---------------------------------- //


        // We create a thread that reads data from the server.
        // The thread is detached so that it runs independently of the main thread.
        // read_from_server(ssl_socket) - this is the fn we made above - above, we got the ssl_socket.
        std::thread(read_from_server, std::ref(ssl_socket)).detach();
        // When a thread is detached, it becomes a detached thread, meaning it runs independently from the main program.
        // std::ref() - this is used to pass the ssl_socket object by reference to the thread.
        // I asked ChatGPT why we couldn't use &ssl_socket instead of std::ref(ssl_socket) - there was a long explanation. We can use, but then we need to change the function signature to take a reference to the ssl_socket object - this didn't make too much sense to be honest.

        // We create a string object to store the message that we want to send to the server.
        // We then use a while loop to keep reading messages from the user and sending them to the server.
        // message gets over written all the time - so we can keep sending messages.
        std::string message;

        // We have an indefinite loop that keeps on reading messages from the user and sending them to the server.
        while (std::getline(std::cin, message))
        {
            // We write to the server using the ssl_socket object.
            // The write function writes the message to the SSL socket.
            boost::asio::write(ssl_socket, boost::asio::buffer(message));

            // std::cout << "Sent message: " << message << std::endl;
            // std::cout << "Message sent to server." << std::endl; // Sunday 02 March 2025 0058 Don't need to print this message - it's not needed.
        }
        // I asked ChatGPT why this loop wasn't detached like the one above. The answer was that the loop involves a cin - this in int's nature is a blocking fn. Trying to detach would apparently cause a lot of complexity. If cin where not involved and something else happend that didn't use somehing similar to cin, then detaching would maybe OK to do. I left it there.
    }
    // If we've had problems making the client, we catch the exception and print an error message.
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

int main(int argc, char *argv[])
{

    /*

    // We check if we have called the function with the correct number of arguments.
    if (argc != 3)
    {
        std::cerr << "Usage example: " << "client" << " <host> <port>\n"
                  << "Or:" << "client" << " localhost 12345\n"
                  << "Or:" << "client" << " 180.1.2.3 45678\n";
        // TODO: replace "client" with argv[0] and strip the string and give just the file name with no exe. Not essential, but would be nice to do with Regex.
        return 1;
    }

    */

    // ---------------------------------- //

    std::cout << "Welcome to the C++ messaging system coded with Boost ASIO using SSL/TLS" << std::endl << std::endl;

    std::string ip_address;
    std::string port_number;
    
    // ---------------------------------- //
    // Added Sunday 02 March 2025 0009
    std::string name;

    std::cout << "Please enter your name:" << std::endl;
    std::cin >> name;

    std::cout << std::endl << std::endl;
    // ---------------------------------- //

    std::cout << "Please enter the IP address of the server you want to connect to:" << std::endl << std::endl;
    std::cin >> ip_address;

    std::cout << std::endl << std::endl;

    std::cout << "Please enter the port number of the server you want to connect to:" << std::endl << std::endl;
    std::cin >> port_number;

    std::cout << std::endl << std::endl;
    std::cout << "ip_address: " << ip_address;
    
    std::cout << std::endl << std::endl;

    std::cout << "port_number: " << port_number;

    std::cout << std::endl << std::endl;

    // ---------------------------------- //


    // We create an io_context object. This object is used to manage the I/O services. It's the main big boss that runs the show 😎
    boost::asio::io_context io_context;

    // We create an ssl_context object. This object is used to manage the SSL context.
    boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);

    // We load the certificate and private key files into the ssl_context object.
    // ssl_context.load_verify_file("ssl_certification/certificate.crt");
    try
    {
        ssl_context.load_verify_file("ssl_certification/certificate.crt");
        std::cout << "Certificate loaded successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading certificate: " << e.what() << std::endl;
    }

    // We call the client function with the io_context object, the ssl_context object, the host name, and the port number.
    // client(io_context, ssl_context, argv[1], std::atoi(argv[2]));
    // client(io_context, ssl_context, ip_address, std::stoi(port_number)); Sunday 02 March 2025 0030
    client(io_context, ssl_context, ip_address, std::stoi(port_number), name);

    return 0;
}

// Build: g++ more_advanced_boost.cpp -o more_advanced_boost -lboost_system -pthread
// Run: ./more_advanced_boost 1234
// g++ -o boost_hello_world boost_hello_world.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread
// This also works:
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto
// ChatGPT tells me to add -pthread to the command line - its says it's necessary to support multithreading.
// It says it might work even without adding the flag because the compiler might add it automatically. Best to be safe and add I think.

// VSCODE prompted this as well:
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread -std=c++11
// g++ -o client client.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread -std=c++11

/*


*/