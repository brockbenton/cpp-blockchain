// simple_client.cpp:
// 1. Creates a socket
// 2. Connects to the server (localhost:8080)
// 3. Sends a message

#include <iostream>
#include <string> 
#include <cstring> // for memset
#include <unistd.h> // for close()
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> // for sockaddr_in

int main() {
    // 1. Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // 2. Setup server address, connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // localhost
    
    connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    std::cout << "Connected to server!" << std::endl;
    
    // 3. Send message
    std::string message = "Hello from client!";
    send(clientSocket, message.c_str(), message.length(), 0);
    std::cout << "Message sent!" << std::endl;
    
    close(clientSocket);
    
    return 0;
}