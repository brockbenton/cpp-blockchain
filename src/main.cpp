#include "Block.h"
#include "Blockchain.h"
#include "Node.h"
#include "Transaction.h"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

void printMenu() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "       BLOCKCHAIN NODE MENU" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Create transaction" << std::endl;
    std::cout << "2. Mine pending transactions" << std::endl;
    std::cout << "3. View blockchain" << std::endl;
    std::cout << "4. Check balance" << std::endl;
    std::cout << "5. Validate chain" << std::endl;
    std::cout << "6. Connect to peer" << std::endl;
    std::cout << "7. View network info" << std::endl;
    std::cout << "8. Save blockchain" << std::endl;
    std::cout << "9. Load blockchain" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Choice: ";
}

int main() {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║    DISTRIBUTED BLOCKCHAIN NODE         ║" << std::endl;
    std::cout << "║    C++ Implementation                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝\n" << std::endl;
    
    // Get node configuration
    int port, difficulty;
    double miningReward;
    
    std::cout << "Enter port for this node (e.g., 8080): ";
    std::cin >> port;
    
    std::cout << "Enter mining difficulty (recommended: 2-4): ";
    std::cin >> difficulty;
    
    std::cout << "Enter mining reward (e.g., 50): ";
    std::cin >> miningReward;
    
    // Create and start node
    Node node(port, difficulty, miningReward);
    node.start();
    
    std::cout << "\n✓ Node started on port " << port << std::endl;
    std::cout << "✓ Chain initialized with genesis block" << std::endl;
    
    // Pending transactions
    std::vector<Transaction> pendingTransactions;
    
    bool running = true;
    while (running) {
        printMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1: {
                // Create transaction
                std::string from, to;
                double amount;
                
                std::cout << "\n--- Create Transaction ---" << std::endl;
                std::cout << "From: ";
                std::getline(std::cin, from);
                std::cout << "To: ";
                std::getline(std::cin, to);
                std::cout << "Amount: ";
                std::cin >> amount;
                
                Transaction tx(from, to, amount);
                pendingTransactions.push_back(tx);
                
                std::cout << "✓ Transaction added to pending pool (" 
                          << pendingTransactions.size() << " pending)" << std::endl;
                break;
            }
            
            case 2: {
                // Mine block
                if (pendingTransactions.empty()) {
                    std::cout << "\n⚠ No pending transactions to mine!" << std::endl;
                } else {
                    std::cout << "\n⛏ Mining block with " << pendingTransactions.size() 
                              << " transactions..." << std::endl;
                    
                    node.mineAndBroadcast(pendingTransactions);
                    pendingTransactions.clear();
                    
                    std::cout << "✓ Block mined and broadcast to network!" << std::endl;
                }
                break;
            }
            
            case 3: {
                // View blockchain
                std::cout << "\n--- Blockchain ---" << std::endl;
                node.getBlockchain().printChain();
                break;
            }
            
            case 4: {
                // Check balance
                std::string address;
                std::cout << "\nEnter address: ";
                std::getline(std::cin, address);
                
                double balance = node.getBlockchain().getBalance(address);
                std::cout << "\n💰 Balance of " << address << ": " << balance << std::endl;
                break;
            }
            
            case 5: {
                // Validate chain
                std::cout << "\n--- Chain Validation ---" << std::endl;
                bool valid = node.getBlockchain().isChainValid();
                
                if (valid) {
                    std::cout << "✓ Blockchain is VALID" << std::endl;
                } else {
                    std::cout << "✗ Blockchain is INVALID - tampering detected!" << std::endl;
                }
                break;
            }
            
            case 6: {
                // Connect to peer
                std::string address;
                int peerPort;
                
                std::cout << "\n--- Connect to Peer ---" << std::endl;
                std::cout << "Peer IP (e.g., 127.0.0.1): ";
                std::getline(std::cin, address);
                std::cout << "Peer port: ";
                std::cin >> peerPort;
                
                if (node.connectToPeer(address, peerPort)) {
                    std::cout << "✓ Connected to peer!" << std::endl;
                } else {
                    std::cout << "✗ Failed to connect to peer" << std::endl;
                }
                break;
            }
            
            case 7: {
                // Network info
                std::cout << "\n--- Network Information ---" << std::endl;
                std::cout << "Port: " << port << std::endl;
                std::cout << "Chain length: " << node.getBlockchain().getChainLength() << " blocks" << std::endl;
                std::cout << "Difficulty: " << difficulty << std::endl;
                std::cout << "Mining reward: " << miningReward << std::endl;
                std::cout << "Pending transactions: " << pendingTransactions.size() << std::endl;
                break;
            }
            
            case 8: {
                // Save blockchain
                std::string filename;
                std::cout << "\nEnter filename (e.g., blockchain.json): ";
                std::getline(std::cin, filename);
                
                if (node.getBlockchain().saveToFile(filename)) {
                    std::cout << "✓ Blockchain saved to " << filename << std::endl;
                } else {
                    std::cout << "✗ Failed to save blockchain" << std::endl;
                }
                break;
            }
            
            case 9: {
                // Load blockchain
                std::string filename;
                std::cout << "\nEnter filename: ";
                std::getline(std::cin, filename);
                
                if (node.getBlockchain().loadFromFile(filename)) {
                    std::cout << "✓ Blockchain loaded from " << filename << std::endl;
                    node.getBlockchain().printChain();
                } else {
                    std::cout << "✗ Failed to load blockchain" << std::endl;
                }
                break;
            }
            
            case 0: {
                // Exit
                std::cout << "\n👋 Shutting down node..." << std::endl;
                node.stop();
                running = false;
                break;
            }
            
            default:
                std::cout << "\n⚠ Invalid choice!" << std::endl;
        }
    }
    
    std::cout << "\n✓ Node stopped. Goodbye!" << std::endl;
    return 0;
}