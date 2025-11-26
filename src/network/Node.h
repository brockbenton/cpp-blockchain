// A Node combines networking + the blockchain.

// A Node should:
// - Have its own blockchain
// - Listen on a port for incoming peer connections
// - Connect to other nodes (peers)
// - Send messages to peers (new blocks, chain requests, etc.)
// - Receive messages from peers
// - Run in the background using threads

#ifndef NODE_H
#define NODE_H

#include "Blockchain.h"
#include <string>
#include <vector>
#include <thread> // For background threads
#include <mutex> // For thread-safe blockchain access
#include <atomic> // For the running blag

class Node {
    private:
        Blockchain blockchain;
        int port; // Port this node listens on
        int serverSocket; // Socket for accepting connections
        std::vector<int> peerSockets; // Connected peers
        std::thread listenerThread; // Background thread for listening
        std::mutex chainMutex; // Protect blockchain from concurrent access
        std::mutex peersMutex;  // Protect peerSockets vector
        bool running; // Is node running?
    
    public:
        // Constructor
        Node(int port, int difficulty, double miningReward);

        // Destructor
        ~Node();

        // Start the node (begin listening for connections)
        void start();

        // Stop the node
        void stop();

        // Connect to another node
        bool connectToPeer(const std::string& address, int port);

        // Mine a new block and broadcast it
        void mineAndBroadcast(std::vector<Transaction> transactions);

        // Get blockchain (for printing/testing)
        Blockchain& getBlockchain();

    private:
        // Background thread that listens for connections
        void listenForConnections();

        // Handle an individual peer connection
        void handlePeer(int peerSocket);

        // Broadcast a message to all connected peers
        void broadcastMessage(const std::string& message);

        // Request chain from peer
        void requestChainFromPeer(int peerSocket);

        // Sync with peer (adopt longer chain)
        void syncWithPeer(int peerSocket);

        // Send our chain to a peer
        void sendChain(int peerSocket);

        // Receive chain from a peer
        void receiveChain(const std::string& message, int peerSocket);

        // Receive a new block from a peer
        void receiveBlock(const std::string& message);

        // Send our chain length to a peer
        void sendLength(int peerSocket);
};

#endif