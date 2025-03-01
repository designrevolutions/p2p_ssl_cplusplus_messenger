#include <iostream>
#include <boost/asio.hpp>

int main() {
    boost::asio::io_context io_context; // Required for asio
    boost::asio::steady_timer timer(io_context, boost::asio::chrono::seconds(1));  // One-second timer

    timer.async_wait([](const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Hello, Boost.Asio!" << std::endl;
        } else {
            std::cerr << "Timer error: " << error.message() << std::endl;
        }
    });

    io_context.run(); // Run the event loop

    return 0;
}