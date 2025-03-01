#include <boost/asio.hpp>
#include <iostream>

int main() {
    // Create an io_context object
    boost::asio::io_context io;

    // Print a message to verify Boost.Asio is working
    std::cout << "Boost.Asio is working!" << std::endl;

    return 0;
}
    
/*
    To compile the program, run the following command: 
    g++ -std=c++11 -I /path/to/boost_1_76_0 boost_hello_world.cpp -o boost_hello_world -lboost_system
    
    Replace  /path/to/boost_1_76_0  with the path to the Boost library on your system. 
    Run the program by executing the following command: 
    ./boost_hello_world
    
    If everything is set up correctly, you should see the following output: 
    Boost.Asio is working!
    
    Conclusion 
    You have successfully installed the Boost C++ libraries on your Ubuntu 20.04 system. You can now use the Boost libraries in your C++ projects. 
    If you have any questions or feedback, feel free to leave a comment. 
    Boost is a set of libraries for the C++ programming language that provides support for tasks and structures such as linear algebra, pseudorandom number generation, multithreading, image processing, regular expressions, and unit testing. 
    To install the Boost C++ libraries on Ubuntu 20.04, follow these steps: 
    First, update the package list and install the required dependencies: 
    sudo apt update
*/