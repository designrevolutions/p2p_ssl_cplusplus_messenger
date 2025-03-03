#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <algorithm>

using boost::asio::ip::tcp;

// We create a session class. This session class is a class that represents a connection to each client.
// Everytime we get a new connection, we create a new session object. This new session object is managed by a std::shared_ptr.
// std::enable_shared_from_this is a CLASS TEMPLATE that enables objects of derived classes to be shared with std::shared_ptr.
// It is used as a base class for classes that are designed to be managed by shared_ptr objects.
// It is used to create a shared_ptr that manages the lifetime of the object pointed to by this.
// std::enable_shared_from_this is a built in part of C++ - this is important to know and can be a cause of confusion for beginners not familiar with C++.
// The 'Session' class is a user declared class that is derived from the C++ template std::enable_shared_from_this.
// 'Session' class can be shared with std::shared_ptr objects.
// shared_from_this() is what is called to get that std::shared_ptr.
// NOTE: public inheritance makes it clear that Session is using the features of std::enable_shared_from_this explicitly and openly.
// Without public inheritance, the std::enable_shared_from_this features would not be available to the Session class.
class Session : public std::enable_shared_from_this<Session>
{
public:
    // We create a constructor for the Session class.
    // The constructor takes a boost::asio::ssl::stream<tcp::socket> object and a std::vector<std::shared_ptr<Session>> object as arguments.
    // boost::asio::ssl::stream<tcp::socket> is the SSL socket that is used to communicate with the client.
    // std::vector<std::shared_ptr<Session>> is a vector that holds all the sessions that are currently connected to the server.
    // The constructor initialises the ssl_socket_ member variable with the SSL socket object that is passed in as an argument.
    // Later in the code we will declare a vector to hold all the sessions that are connected to the server.
    Session(boost::asio::ssl::stream<tcp::socket> socket,
            std::vector<std::shared_ptr<Session>> &sessions)
        : ssl_socket_(std::move(socket)), sessions_(sessions) {}

    // ---------------------------------- //

    // Now we create public member functions for the Session class - availabe to everything to call without limits.

    // This gets called when we start the session.
    void start()
    {
        sessions_.push_back(shared_from_this()); // Here we add the session to the sessions vector.

        print_connected_clients(); // We print the connected clients.

        do_handshake(); // We do the SSL handshake.
    }

private:
    // This function prints the connected clients.
    // It goes through all the sessions that are currently connected to the server and prints the IP address of the client that is connected on + the port number.
    void print_connected_clients()
    {
        std::cout << "Connected clients:" << std::endl;

        for (const auto &session : sessions_) // I prefer &session instead of auto& session - but it's the same thing.
        {
            std::cout << session->ssl_socket_.lowest_layer().remote_endpoint() << std::endl;
        }

        std::cout << std::endl;
    }

    // This function does the SSL handshake. This is standard procedure when using SSL with boost code.
    void do_handshake()
    {
        // We create a shared pointer to the current session object.
        // This is needed because the async_handshake function requires a shared pointer to the current object.
        auto self(shared_from_this());

        // This is the main function that does the SSL handshake.
        // The async_handshake function is an asynchronous function that performs an SSL handshake on the stream.
        ssl_socket_.async_handshake(
            boost::asio::ssl::stream_base::server,

            [this, self](boost::system::error_code ec)
            {
                if (!ec)
                {
                    std::cout << "Server side: SSL handshake completed successfully with client." << std::endl << std::endl;
                    read();
                }
                else
                {
                    // If we have an error in the handshake, we remove the session from the sessions vector.
                    // Later, in future versions, we can add code to try to reconnect and try connecting again x number of times before erasing.
                    sessions_.erase(
                        std::remove(sessions_.begin(), sessions_.end(), self),
                        sessions_.end());
                    print_connected_clients();
                }
            });
    }

    // This function reads data from the client.
    // It reads data from the SSL socket and stores it in the data_ member variable.
    // The function then prints the data that was read and calls the broadcast function.
    // The broadcast function sends the data to all the other clients that are connected to the server.
    // The function then calls itself again to read more data from the client.
    // It will keep on reading data from the client until there is no data left.
    // When there is no data left, an error is raised.
    void read()
    {
        auto self(shared_from_this());

        ssl_socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    [this, self](boost::system::error_code ec, std::size_t length)
                                    {
                                        if (!ec)
                                        {
                                            /* Sunday 02 March 2025 0035
                                            // We print the data that was received from the client.
                                            // TODO - add the name of the client to the message.
                                            std::cout << "Received message: " << std::string(data_, length) << std::endl;

                                            broadcast(length);
                                            read();
                                            */

                                            // ---------------------------------- //
                                            // Added Sunday 02 March 2025 0039
                                            if (!ec)
                                            {
                                                if (client_name_.empty()) // Name hasn't been set yet
                                                {
                                                    client_name_ = std::string(data_, length);
                                                    std::cout << "Client name received: " << client_name_ << std::endl;
                                                    std::cout << "Welcome " << client_name_ << std::endl << std::endl;
                                                }
                                                else
                                                {
                                                    std::cout << "[" << client_name_ << "]: " << std::string(data_, length) << std::endl;
                                                    broadcast(length);
                                                }
                                                read();
                                            }
                                            // ---------------------------------- //

                                            
                                        }
                                        else
                                        {
                                            sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                                            print_connected_clients();
                                        }
                                    });
    }

    // This function broadcasts data to all the clients that are connected to the server.
    // It goes through all the sessions that are currently connected to the server.
    // If the session is not the current session, it writes the data to the session.
    // The data is written to the session using the write function.
    // The write function writes the data to the session and then calls the read function to read more data from the client.
    // The write function is an asynchronous function that writes data to the SSL socket.
    // I asked why we need to pass length and why we can't use max_length - the answer is that we need to pass the length of the data that we want to write.
    // So for example, if we wrtie "Hello", that is 5 characters long - so we need to pass 5 as the length.

    /* Sunday 02 March 2025 0048 - in the newer version, I add the clients name when they send a message. */
    /*
    void broadcast(std::size_t length)
    {
        for (auto &session : sessions_)
        {
            // We check if the session is not the current session.
            // If it's not the current session, we write the data to the session.
            if (session != shared_from_this())
            {
                session->write(data_, length);
            }
        }
    }
    */

    // This new version of the broadcast function adds the name of the client to the message.
    void broadcast(std::size_t length)
    {
        std::string formatted_message = client_name_ + ": " + std::string(data_, length);
    
        for (auto &session : sessions_)
        {
            // If the session is not the current session, write the formatted message to the session.
            if (session != shared_from_this())
            {
                session->write(formatted_message.c_str(), formatted_message.size());
            }
        }
    }
    

    // Damn this VSCode is good - it's really good - I'm loving it - it's so good - it's really AWESOME! 😎

    void write(const char *data, std::size_t length)
    {
        // We create a shared pointer to the current session object.
        // This is needed because the async_write function requires a shared pointer to the current object.
        // We use shared_from_this() to get a shared pointer to the current object.
        auto self(shared_from_this());

        boost::asio::async_write(ssl_socket_, boost::asio::buffer(data, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         read();
                                     }
                                     else
                                     {
                                         sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                                         print_connected_clients();
                                     }
                                 });
    }

    // This is the SSL socket that is used to communicate with the client.
    // It is a boost::asio::ssl::stream object that is used to communicate with the client.
    // All complexities of the socket and SSL are handled by this object.
    // The SSL socket is a stream that is used to communicate with the client.
    boost::asio::ssl::stream<tcp::socket> ssl_socket_;

    // This is a buffer that is used to store the data that is read from the client.
    // It is a char array that is used to store the data that is read from the client.
    enum
    {
        max_length = 1024
    };

    // This is a char array that is used to store the data that is read from the client.
    // This is our buffer that we use to store the data that is read from the client.
    // Don't get mixed up with the socket buffeer, the socket buffer is a buffer that is used by the socket to store data that is read from the client.
    char data_[max_length];

    // This is a vector that holds all the sessions that are currently connected to the server.
    // Theoretically, this vector should hold all the sessions that are currently connected to the server.
    std::vector<std::shared_ptr<Session>> &sessions_;

    std::string client_name_; // Sunday 02 March 2025 0033 - I need to store the name of the client - this is a bit of a hack I think, I would prefer to pass in as an object

};

// We create a server class.
// The server class is a class that represents the server.
// The server class is used to accept connections from clients and create a new session object for each client that connects.
class Server
{

public:
    // We create a constructor for the Server class.
    // The constructor takes a boost::asio::io_context object, a boost::asio::ssl::context object and a short object as arguments.
    // The boost::asio::io_context object is used to manage the I/O services.
    // The boost::asio::ssl::context object is used to manage the SSL context.
    // The short object is used to store the port number that the server listens on. short is a 16-bit integer - part of C++.
    // The constructor initialises the acceptor_ member variable with the io_context object and the port number that is passed in as arguments.
    Server(boost::asio::io_context &io_context, boost::asio::ssl::context &ssl_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), ssl_context_(ssl_context), sessions_() // sessions_() is declared further below - this is why we can use here. In classes, you can declare something further down and use it before it's declared.
    {
        std::cout << "Message server started. Ready to accept connections..." << std::endl << std::endl;

        accept();
    }
    // Can you remember and recall what the single : does above?
    // The single : is used to initialise the member variables of the class.
    // This is quite unique to C++ and is not found in other programming languages. I couldn't find any other languages that use this method.

    // STOPPED HERE Wednesday 26 February 2025 1520 - need to read up on ssl_context_(ssl_context)

private:
    void accept()
    {
        // The acceptor_.async_accept function is an asynchronous function that accepts connections from clients.
        // It sets up an asynchronous operation that waits for a client to connect and then calls the lambda function that is passed in as an argument.
        // It only triggers when a client connects to the server or when an error occurs.
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::cout << "New client connected!" << std::endl << std::endl;

                    // We create a new session object for the client that has connected.
                    // We use make_shared - make_shared is part of C++ and created a shared_pointer.
                    std::make_shared<Session>(boost::asio::ssl::stream<tcp::socket>(std::move(socket), ssl_context_), sessions_)->start();
                }

                // This is what makes the server keep on accepting connections from clients.
                // This loop sits within the io_context.run(); loop.
                // Normally, recursion could cause a stack overflow - but in this case, it's safe because the accept function is called asynchronously.
                // Boost.Asio and C++ are designed to handle this kind of asynchronous recursion efficiently and safely.
                accept();
            });
        // The function doesn't loop indefinitely in a blocking manner and only sets up an asynchronous operation.
        // The io_context.run() event loop manages the execution, ensuring the handler is called when a client connects.
    }

    // ---------------------------------- //
    // Below we have the member variables of the Server class. These are meant to be private, no need to allow public access to these variables.

    // This is the acceptor that is used to accept connections from clients.
    // The acceptor is a boost::asio::ip::tcp::acceptor object that is used to accept connections from clients.
    // The tcp::acceptor class is used to listen for incoming TCP connections on a specific port.
    // The acceptor_ binds to a specific IP address and port number where it will listen for incoming connections.
    tcp::acceptor acceptor_;

    // The ssl_context_ is a boost::asio::ssl::context object that is used to manage the SSL context.
    // context is a term used in SSL/TLS to refer to the configuration and state of the SSL/TLS connection.
    // It's just an object that we need to use to add SSL/TLS.
    boost::asio::ssl::context &ssl_context_;
    // ssl_context is passed in as a refernece when the server is created. So the ssl_contextt is created before we pass it in.
    // This is done in the main fn.

    // This is a vector that holds all the sessions that are currently connected to the server.
    std::vector<std::shared_ptr<Session>> sessions_;
};

int main(int argc, char *argv[])
{
    try
    {
        // First we check if we have called the function with the correct number of arguments.
        // If we haven't, we print an error message and return 1 and the code exits and doesn't run
        if (argc != 2)
        {
            std::cerr << "Usage: ./server <port>\n";
            return 1;
        }

        // We create an io_context object. This object is used to manage the I/O services. It's the main big boss that runs the show 😎 
        boost::asio::io_context io_context;

        // ---------------------------------- //
        // We create an ssl_context object. This object is used to manage the SSL context.
        // The SSL context is used to manage the SSL configuration and state of the SSL connection.
        boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23);

        ssl_context.use_certificate_chain_file("ssl_certification/certificate.crt");
        ssl_context.use_private_key_file("ssl_certification/private.key", boost::asio::ssl::context::pem);
        // ---------------------------------- //

        // We create a Server object. This object is used to represent the server.
        // The Server object is created with the io_context object, the ssl_context object and the port number that is passed in as arguments.
        Server server(io_context, ssl_context, std::atoi(argv[1]));

        // We run the io_context object. This is the main event loop that runs the server - and all other asynchronous operations.
        // The io_context object is the main boss that runs the show 😎
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

// Build: g++ more_advanced_boost.cpp -o more_advanced_boost -lboost_system -pthread
// Run: ./more_advanced_boost 1234
// g++ -o boost_hello_world boost_hello_world.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32
// g++ -o more_advanced_boost more_advanced_boost.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock
// g++ -o server server.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock
// g++ -o server server.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto

// VSCODE gave this:
// g++ -o server server.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread
// g++ -o server server.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock -lssl -lcrypto -pthread -std=c++11

/*
Usage:

Run MSYS2 UCRT64 for each session needed: 1 for server, 1 for client, another 1 for each client

You should have compiled code as detailed further below.

We start the server like this:

./server 12340

Then we start the client like this:

./client localhost 12340

12340 is the port number. This can be anything - but within limits - there are other port numbers that are reserved that you shouldn't use.

localhost is the IP address the server is on.
This can be the IP address of the server, or localhost if you are running the server on the same machine as the client.
localhost is a special IP address that always refers to the machine you are currently on.
It's also 127.0.0.1.

You can also find out the actual IP address of the machine by running IPConfig in the command prompt. This works on PC and Mac.
For Linux you can use ifconfig - but the modern replacement is the command: "ip".


*/

/*
Feedback from Fowsi:

When client starts: ask for name

Once server starts: what's it doing? Add message saying: Awaiting for clients ✔️ 

SSL - use Open SSL ✔️ 

Add Github repositiry ✔️ 

Make youtube video going through the code

Need to give messages like: estalibslihed Open SSL

*/

/*
Need to do:

- Get SSL working ✔️ 
- Split code into multiple files and organise
- Rewrite all code and understand every single part ✔️ 
- Add keywords to exit the server and client ❌ Will do later
- Add a way for clients to choose color?
- Get colour coding
- Put code into one code set - so the code can be run using one command - like 'cpp_p2p.exe then ading a flag to say if it's a server or client.
- Add more advanced features
- Make a GUI version of the code
- Add ability to send files - MUST MUST get this done ASAP

*/

/*
How to generate SSL certificates:

Install OpenSSL:

- MSYS2: pacman -Sy (to update the package database)
- MSYS2: pacman -S mingw-w64-x86_64-openssl (to install OpenSSL)
- MSYS2: openssl genrsa -out private.key 2048 (now you get a file called private.key)
- MSYS2: openssl req -new -key private.key -out server.csr (now you get a file called server.csr) (you will be asked for some information)
- MSYS2: openssl x509 -req -days 365 -in server.csr -signkey private.key -out server.crt (now you get a file called server.crt - this is a self signed certificate - normally, you would get this from a certificate authority)
- MSYS2: openssl pkcs12 -export -out server.pfx -inkey private.key -in server.crt (now you get a file called server.pfx) (Not sure what this is - VS Code says it's a PKCS #12 file - but I don't know what that is)

#########
VS Code gave me this (this I didn't do, I thought I would leave the comment incase I wanted to do it later):

I made a file called openssl.bat with the following content:

@echo off
openssl genrsa -out private.key 2048
openssl req -new -key private.key -out server.csr
openssl x509 -req -days 365 -in server.csr -signkey private.key -out server.crt
openssl pkcs12 -export -out server.pfx -inkey private.key -in server.crt

Then I run the file by typing openssl.bat in the command prompt.
#########

I made a file called openssl.cnf with the following content:

----------------------------------
[ req ]
default_bits       = 2048
default_keyfile    = private.key
distinguished_name = DesignRevolutions
prompt             = no

[ DesignRevolutions ]
C            = UK
ST           = London
L            = LondonCity
O            = DesignRev
OU           = IT
CN           = my_localhost
emailAddress = o@designrevolution.com
----------------------------------

Then I run the following commands:

openssl genrsa -out private.key 2048

openssl req -new -x509 -days 365 -key private.key -out server.crt -config openssl.cnf

*/

/*
TODO:

Each client should have a name - so when a client sends a message, it should be prefixed with the name of the client.
Within the code add a new variable called name - this will be passed in as an argument when the client is started.
When the client is started, it should ask for the name of the client.
When the client sends a message, it should prefix the message with the name of the client.

Each client should have a different colour - so when a client sends a message, it should be in a different colour.
Make the colour change in both the server and the client side. Do client side first.

Ver1:
The server holds x colours in an array.
When connecting to the server, the client gets returned a colour to use on the client side.
The client then use this colour to display the messages in the client side.

Ver2:
The client side chooses a random colour for it's own messages.
All other messages are white. We don't have to choose a random color.
Keep it simple first. Get it working first.

*/

/*
TODO:

Add a keyword, so if the client types EXIT in big capital letters, the client will exit.
This would be easy to do.

*/

/*
TODO: wrtie pseudo code for everything

TODO: another smaller pseudo code version: write a plan for everything - make the whole code be 10 chunks of pseudo code

*/

/*
When a client exits, the server should print a message saying that the client has exited and give meaningful information.
Like: John has exited the chat.
*/

/*
TODO:

Add a countdown timer to the server - so it can be shut down after a certain amount of time. So if no clients are connected after a certain amount of time, the server will shut down.

Add a countdown timer to the client - so it can be shut down after a certain amount of time - so if the client isn't connected to the server after a certain amount of time, it will shut down.

Let's say we set the timer to be 10 minutes, when there is 60 seconds left, the server will send a message to the client saying that the server will shut down in 60 seconds and a timer will start counting down.

Loving this auto fill feature in VS Code - it's really good. 😎
*/

/*
Add other keywords:

WHO - this will show all the clients that are connected to the server and give other details like IP and port connected on.


*/

/*
If the client tries to connect and there is no server running, the client should print a message saying that the server is not running and then exit.

*/

/*
TODO:

Small bug in the code...
When a client is diconnected, there are 2 printout of the connected clients.
Same when no clients are connected.
*/

/*
TODO:

Would be nice to have: show the raw encrypted data that is sent between the client and the server and then on the line below show the decrypted data.

*/

/*
TODO:

Can add an option if the server should accept new clients or not.

*/

/*
TODO:

How can we add emjois to the code?
Need to change the alphabet to include emojis.

*/


/*
TODO (but not urgent):

Add a way for the server to send a message to all the clients.

Instead of ending sessions on error, we can try to reconnect. Allow clients to attempt to reconnect 3 times before ending the session.

*/

/*
TODO (in client code): replace "client" with argv[0] and strip the string and give just the file name with no exe. Not essential, but would be nice to do with Regex.
*/

/*
TODO: allow for the serve to decline accepting an incoming connection.
We can block on IP address and/or name.
*/

/*
TODO: assign a different colour to each client. So if we have 5 people chatting, then each person has a different colour.

Currently code has been added so that we have green olive colour for others who are chatting. This is a good start.

*/

/*
TODO:
Need to have some basic error checking of user input for host and port number and even name.

*/