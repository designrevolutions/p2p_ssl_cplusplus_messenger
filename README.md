# p2p_ssl_cplusplus_messenger
C++ P2P SSL Messenger - using Boost ASIO

This is a C++ Project developing a C++ P2P messenger application.
The code uses Boost ASIO.

The core files are: 

client.cpp
server.cpp
ssl_certification folder

Everthing else is not needed and there for other purposes - like backup files I kep when developing on my laptop (previous to using Git and Github).

The code was developed in Visual Studio Code. Oh damn, that gave me so many problems. I'll try and document my troubles later.

A very important key part of compiling the exe files AND before that installing Boost and Boost ASIO is using MSYS2 terminal.

---------------------------------- 
How to use:

Compile the C++ Code
(I'll give more instructions on this later - instruction commands are gioven at the end of the files. I have been compiling on the command line.)

Run the server, then run clients.

Running the server on the MSYS2 command line:

./server 12345

(12345 is a port number - it can be any number. Any number apart from reserved ports I guess and within the limits.)

Running clients (after the server has been started), there are 2 ways.

Version 1:

./client

You will now be prompted to answer questions where you give the host server IP address, port number and you name.

Version 2:

./client 127.0.0.1 12345 Omar

First give the host IP address, second the port number (must be the same as the server) and then your name.

There is some basic error checking to make sure you enter the right amounf of arguments.