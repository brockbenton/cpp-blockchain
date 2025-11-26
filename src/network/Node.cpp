#include "Node.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Node::Node(int port, int difficulty, double miningReward)
    : blockchain(difficulty, miningReward), port(port), running(false) {
    serverSocket = -1;
}

Node::~Node() {
    stop(); // Clean up
}

Blockchain& Node::getBlockchain() {
    return blockchain;
}

void Node::start() {
    // 1. Create a socket (like in simple_server)
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Allow port reuse (for testing)
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Setup address and bind (like in simple_server)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 3. Start listening
    listen(serverSocket, 5);
    std::cout << "Node listening on port " << port << "..." << std::endl;

    // 4. Set running flag
    running = true;

    // 5. Launch background thread
    listenerThread = std::thread(&Node::listenForConnections, this);
    
    std::cout << "Node started on port " << port << std::endl;
}

void Node::listenForConnections() {
    while (running) {
        // Accept a connection (this blocks until someone connects)
        int peerSocket = accept(serverSocket, nullptr, nullptr);

        if (peerSocket < 0) {
            continue;
        }

        std::cout << "Peer connected!" << std::endl;

        // Add to our list of peers
        peerSockets.push_back(peerSocket);

        // Handle this peer in another thread
        std::thread(&Node::handlePeer, this, peerSocket).detach();
    }
}

void Node::stop() {
    running = false;

    // Close server socket
    if (serverSocket >= 0) {
        close(serverSocket);
    }

    // Close all peer connections
    for (int sock : peerSockets) {
        close(sock);
    }

    // Wait for listener thread to finish
    if (listenerThread.joinable()) {
        listenerThread.join();
    }

    std::cout << "Node stopped" << std::endl;
}

bool Node::connectToPeer(const std::string& address, int port) {
    // 1. Create socket
    int peerSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Setup peer address
    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(port);

    // Convert address string to binary
    if (inet_pton(AF_INET, address.c_str(), &peerAddr.sin_addr) <= 0) {
        std::cout << "Invalid address: " << address << std::endl;
        return false;
    }

    // 3. Connect to peer
    if (connect(peerSocket, (sockaddr*)&peerAddr, sizeof(peerAddr)) < 0) {
        std::cout << "Connection to peer failed: " << address << ":" << port << std::endl;
        return false;
    }

    std::cout << "Connected to peer: " << address << ":" << port << std::endl;

    // 4. Add to peer list
    peerSockets.push_back(peerSocket);

    // 5. Handle this peer in another thread
    std::thread(&Node::handlePeer, this, peerSocket).detach();

    // 6. Sync chains immediately
    syncWithPeer(peerSocket);

    return true;
}

void Node::handlePeer(int peerSocket) {
    char buffer[4096];

    while (running) {
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));

        // Receive message
        int bytesRead = recv(peerSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            // Peer disconnected
            std::cout << "Peer disconnected" << std::endl;
            close(peerSocket);
            
            // Remove from peerSockets vector
            for (size_t i = 0; i < peerSockets.size(); i++) {
                if (peerSockets[i] == peerSocket) {
                    peerSockets.erase(peerSockets.begin() + i);
                    break;
                }
            }
    
            break;
        }

        std::string message(buffer);
        std::cout << "Received: " << message << std::endl;

        // Parse message type
        // Simple parsing: look for "type":"XXX"

        if (message.find("\"type\":\"GET_CHAIN\"") != std::string::npos) {
            // Peer wants our chain
            sendChain(peerSocket);
        }
        else if (message.find("\"type\":\"CHAIN\"") != std::string::npos) {
            // Peer sent us their chain
            receiveChain(message, peerSocket);
        }
        else if (message.find("\"type\":\"NEW_BLOCK\"") != std::string::npos) {
            // Peer mined a new block
            receiveBlock(message);
        }
        else if (message.find("\"type\":\"GET_LENGTH\"") != std::string::npos) {
            // Peer wants to know our chain length
            sendLength(peerSocket);
        }
        else if (message.find("\"type\":\"LENGTH\"") != std::string::npos) {
            // Extract the length value from JSON
            size_t pos = message.find("\"value\":") + 8;
            size_t end = message.find_first_of(",}", pos);
            int peerLength = std::stoi(message.substr(pos, end - pos));
            
            std::cout << "Peer has chain length: " << peerLength << std::endl;
            
            // Compare to our chain length
            chainMutex.lock();
            int ourLength = blockchain.getChain().size();
            chainMutex.unlock();
            
            std::cout << "Our chain length: " << ourLength << std::endl;
            
            // If their chain is longer, request it
            if (peerLength > ourLength) {
                std::cout << "Peer has longer chain! Requesting..." << std::endl;
                std::string request = "{\"type\":\"GET_CHAIN\"}";
                send(peerSocket, request.c_str(), request.length(), 0);
            }
        }
    }
}

void Node::sendChain(int peerSocket) {
    chainMutex.lock();

    // Serialize blockchain to JSON
    std::string chainJson = blockchain.toJSON();

    // Create message
    std::string message = "{\"type\":\"CHAIN\",\"data\":" + chainJson + "}";

    chainMutex.unlock();

    // Send
    send(peerSocket, message.c_str(), message.length(), 0);
}


void Node::receiveChain(const std::string& message, int peerSocket) {
    // Parse chain JSON to Blockchain object
    size_t pos = message.find("\"data\":") + 7;
    size_t end = message.find_last_of("}");
    std::string chainJson = message.substr(pos, end - pos);

    std::vector<Block> loadedBlocks;
    size_t blockStart = 0;
    int braceCount = 0;

    for (size_t i = 0; i < chainJson.length(); i++) {
        char c = chainJson[i];

        if (c == '{') {
            if (braceCount == 0) {
                blockStart = i;
            }
            braceCount++;
        }
        else if (c == '}') {
            braceCount--;

            if (braceCount == 0) {
                std::string blockJson = chainJson.substr(blockStart, i - blockStart + 1);

                Block block = Block::fromJSON(blockJson);
                loadedBlocks.push_back(block);
            }
        }
    }

    // Check if loaded chain is longer and valid
    chainMutex.lock();
    if (loadedBlocks.size() > blockchain.getChain().size()) {
        if (blockchain.isValidChain(loadedBlocks)) {
            blockchain.replaceChain(loadedBlocks);
            std::cout << "Replaced our chain with peer's longer valid chain!" << std::endl;
        } else {
            std::cout << "Received chain is invalid!" << std::endl;
        }
    } else {
        std::cout << "Received chain is not longer than our current chain." << std::endl;
    }
    chainMutex.unlock();
}

void Node::receiveBlock(const std::string& message) {
    // Extract block JSON
    // Parse to Block object
    // Add to our chain if valid

    size_t pos = message.find("\"data\":") + 7;
    size_t end = pos;
    int braceCount = 0;

    for (size_t i = pos; i < message.length(); i++) {
        if (message[i] == '{') braceCount++;
        if (message[i] == '}') {
            braceCount--;
            if (braceCount == 0) {
                end = i + 1;
                break;
            }
        }
    }

    std::string blockJson = message.substr(pos, end - pos);

    Block block = Block::fromJSON(blockJson);

    if (block.index == blockchain.getChain().size()) {
        if (block.previousHash == blockchain.getChain()[block.index - 1].hash) {
            std::string target = "";
            for (int i = 0; i < blockchain.getDifficulty(); i++) {
                target += "0";
            }
            if (block.hash.substr(0, blockchain.getDifficulty()) == target) {
                if (block.hash == block.calculateHash()) {
                    chainMutex.lock();
                    blockchain.addExistingBlock(block);
                    chainMutex.unlock();

                    std::cout << "Added a new block from peer!" << std::endl;
                }
            }
        }
    }
}

void Node::sendLength(int peerSocket) {
    chainMutex.lock();
    int length = blockchain.getChain().size();
    chainMutex.unlock();

    std::string message = "{\"type\":\"LENGTH\",\"value\":" + std::to_string(length) + "}";
    send(peerSocket, message.c_str(), message.length(), 0);
}

void Node::syncWithPeer(int peerSocket) {
    // 1. Ask peer for their chain length
    std::string request = "{\"type\":\"GET_LENGTH\"}";
    send(peerSocket, request.c_str(), request.length(), 0);
    
    // 2. Wait for response and compare
    // (This is simplified - in reality you'd need async handling)
    
    // 3. If their chain is longer, request full chain
    request = "{\"type\":\"GET_CHAIN\"}";
    send(peerSocket, request.c_str(), request.length(), 0);
}

void Node::broadcastMessage(const std::string& message) {
    // Lock to safely read peerSockets
    std::lock_guard<std::mutex> lock(peersMutex);

    for (int sock : peerSockets) {
        if (sock >= 0) {
            send(sock, message.c_str(), message.length(), 0);
        }
    }

    std::cout << "Broadcasting to " << peerSockets.size() << " peers" << std::endl;
}

void Node::mineAndBroadcast(std::vector<Transaction> transactions) {
    std::cout << "Mining new block with " << transactions.size() << " transactions..." << std::endl;
    
    chainMutex.lock();

    blockchain.addBlock(transactions);
    Block& newBlock = blockchain.getChain().back();
    std::string blockJson = newBlock.toJSON();

    chainMutex.unlock();

    std::string message = "{\"type\":\"NEW_BLOCK\",\"data\":" + blockJson + "}";
    broadcastMessage(message);


    std::cout << "Block mined and broadcast!" << std::endl;
}

void Node::requestChainFromPeer(int peerSocket) {
    std::string message = "{\"type\":\"GET_CHAIN\"}";
    send(peerSocket, message.c_str(), message.length(), 0);
    std::cout << "Requested chain from peer" << std::endl;
}

