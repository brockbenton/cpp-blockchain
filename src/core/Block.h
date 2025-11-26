#ifndef BLOCK_H
#define BLOCK_H

#include "Transaction.h"
#include <string>
#include <ctime>
#include <vector>

class Block {
    public: 
        int index;
        std::string previousHash; // link to previous block
        std::string hash; // unique identifier
        std::vector<Transaction> transactions;
        std::time_t timestamp; // time of creation
        int nonce; // used for proof-of-work

        // Constructor
        Block(int idx, std::string prevHash, std::vector<Transaction> txs);

        // Calculate the hash of the block
        std::string calculateHash() const;

        // Mine the block by finding a valid hash
        void mineBlock(int difficulty);

        // Add a transaction to the block
        void addTransaction(Transaction tx);

        // Combine all transactions for hashing
        std::string getTransactionsAsString() const;

        // Convert block to JSON format
        std::string toJSON() const;

        // Parse block from JSON format
        static Block fromJSON(const std::string& json);
};

#endif 