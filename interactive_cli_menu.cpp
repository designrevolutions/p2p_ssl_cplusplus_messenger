#include <iostream>

// In this code I ask the user questions and take in variables and store the input they type in and then I output it back to them.

int main()
{
    std::cout << "Welcome to the C++ messaging system coded with Boost ASIO using SSL/TLS" << std::endl << std::endl;

    std::string ip_address;
    std::string port_number;

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

    return 0;
}
