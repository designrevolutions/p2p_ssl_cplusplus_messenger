// Include necessary libraries and headers
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <algorithm>

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::vector<std::shared_ptr<Session>>& sessions)
        : socket_(std::move(socket)), sessions_(sessions) {}

    void start() {
        sessions_.push_back(shared_from_this());
        read();
    }

private:
    void read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    broadcast(length);
                    read();
                } else {
                    // Remove the session on error (client disconnected)
                    sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                }
            });
    }

    void broadcast(std::size_t length) {
        for (auto& session : sessions_) {
            if (session != shared_from_this()) {
                session->write(data_, length);
            }
        }
    }

    void write(const char* data, std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    read();
                } else {
                    // Remove the session on error
                    sessions_.erase(std::remove(sessions_.begin(), sessions_.end(), self), sessions_.end());
                }
            });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::vector<std::shared_ptr<Session>>& sessions_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), sessions_() {
        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), sessions_)->start();
                }
                accept();
            });
    }

    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        Server server(io_context, std::atoi(argv[1]));
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}


// Build: g++ more_advanced_boost.cpp -o more_advanced_boost -lboost_system -pthread
// Run: ./more_advanced_boost 1234
// g++ -o boost_hello_world boost_hello_world.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32
// g++ -o more_advanced_boost more_advanced_boost.cpp -L/C/Coding/C++/boost_1_87_0/stage/lib -lboost_system-mgw14-mt-x64-1_87 -lboost_filesystem-mgw14-mt-x64-1_87 -lws2_32 -lmswsock