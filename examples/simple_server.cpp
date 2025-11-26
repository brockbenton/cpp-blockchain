// What is a socket?
// A socket is an endpoint for sending or receiving data across a computer network. It is a
// combination of an IP address and a port number, allowing different applications on the
// same device to communicate with each other or with external devices over the network.

// simple_server.cpp:
// 1. Creates a socket
// 2. Binds to a port
// 3. Listens for incoming connections
// 4. Accepts a connection
// 5. Receives data from the client
// 6. Send a response back to the client (prints)

#include <iostream>
#include <string> 
#include <cstring> // for memset
#include <unistd.h> // for close()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

int main() {
    // 1. Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Setup address, bind socket to address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080); // Port 8080
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 3. Listen for connections (max 5 in queue)
    listen(serverSocket, 5);
    std::cout << "Server listening on port 8080..." << std::endl;

    // 4. Accept a connection
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    std::cout << "Client connected!" << std::endl;

    // 5. Receive message
    char buffer[1024] = {0};
    recv(clientSocket, buffer, 1024, 0);
    std::cout << "Received: " << buffer << std::endl;

    // 6. Close sockets
    close(clientSocket);
    close(serverSocket);

    return 0;
}