#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "Block.h"
#include <vector>

class Blockchain {
    public:
        // Constructor to initialize blockchain with given difficulty
        Blockchain(int diff, double reward = 100.0, bool createGenesis = true);

        // Add a new block to the chain
        void addBlock(std::vector<Transaction> tx);

        // Validate the integrity of the blockchain
        bool isChainValid();

        // Print the blockchain
        void printChain();

        // Get block by index
        Block& getBlock(int index);

        // Balance tracking
        double getBalance(std::string address);

        // Check if sender has enough balance
        bool validateTransaction(Transaction tx);

        // Save to file
        bool saveToFile(const std::string& filename) const;

        // Load from file
        bool loadFromFile(const std::string& filename);

        // Convert to JSON
        std::string toJSON() const;

        // Getter
        std::vector<Block>& getChain() { return chain; }

        // Check validity of a given chain
        bool isValidChain(const std::vector<Block>& newChain) const;

        // Replace chain if new one is valid and longer
        void replaceChain(const std::vector<Block>& newChain);

        // Get difficulty
        int getDifficulty() const { return difficulty; }

        // Add existing block
        void addExistingBlock(const Block& block);

        // Get length of chain
        size_t getChainLength() const { return chain.size(); } 

    private:
        std::vector<Block> chain; // The blockchain itself
        int difficulty; // Mining difficulty
        double miningReward; // Reward for mining a block
};

#endif