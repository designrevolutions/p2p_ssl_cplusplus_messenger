#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <algorithm>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, std::vector<std::shared_ptr<Session>> &sessions)
        : socket_(std::move(socket)), sessions_(sessions) {}

    void start()
    {
        sessions_.push_back(shared_from_this());
        print_connected_clients();
        read();
    }

private:
    void read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        std::cout << "Received message: " << std::string(data_, length) << std::endl;
                                        broadcast(length);
                                        read();
                                    }
                                    else
                                    {
                                        // Remove the session on error (client disconnected)
                                        sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                                        print_connected_clients();
                                    }
                                });
    }

    void broadcast(std::size_t length)
    {
        for (auto &session : sessions_)
        {
            if (session != shared_from_this())
            {
                session->write(data_, length);
            }
        }
    }

    void write(const char *data, std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data, length),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         read();
                                     }
                                     else
                                     {
                                         // Remove the session on error
                                         sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                                         print_connected_clients();
                                     }
                                 });
    }

    void print_connected_clients()
    {
        std::cout << "Connected clients:\n";
        for (const auto &session : sessions_)
        {
            std::cout << session->socket_.remote_endpoint() << "\n";
        }
    }

    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
    std::vector<std::shared_ptr<Session>> &sessions_;
};

class Server
{
public:
    Server(boost::asio::io_context &io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), sessions_()
    {
        accept();
    }

private:
    void accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::cout << "New client connected!\n";
                    std::make_shared<Session>(std::move(socket), sessions_)->start();
                }
                accept();
            });
    }

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
};

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: ./server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        Server server(io_context, std::atoi(argv[1]));
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

When client starts: ask for nameOnce server starts: what's it dpinmg? Add message saying: Awaiting for clients

SSL - use Open SSL

Add Github repositiry

Make youtube video going through the code

Need to give messages like: estalibslihed Open SSL

*/

/*
Need to do:

- Get SSL working
- Split code into multiple files and organise
- Rewrite all code and understand every single part
- Add keywords to exit the server and client
- Add a way for clients to choose color?
- Get colour coding
- Put code into one code set - so the code can be run using one command - like 'cpp_p2p.exe then ading a flag to say if it's a server or client.
- Add more advanced features
- Make a GUI version of the code

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
VS Code gave me this:

I made a file called openssl.bat with the following content:

@echo off
openssl genrsa -out private.key 2048
openssl req -new -key private.key -out server.csr
openssl x509 -req -days 365 -in server.csr -signkey private.key -out server.crt
openssl pkcs12 -export -out server.pfx -inkey private.key -in server.crt

Then I run the file by typing openssl.bat in the command prompt.
#########

I made a file called openssl.cnf with the following content:

[ req ]
default_bits       = 2048
default_keyfile    = private.key
distinguished_name = req_distinguished_name
prompt             = no

[ req_distinguished_name ]
C            = US
ST           = California
L            = San Francisco
O            = MyCompany
OU           = MyDivision
CN           = localhost
emailAddress = admin@mycompany.com

Then I run the following commands:

openssl genrsa -out private.key 2048

openssl req -new -x509 -days 365 -key private.key -out server.crt -config openssl.cnf

*/