#include "Block.h"
#include "sha256.h"
#include <iostream>
#include <vector>

Block::Block(int idx, std::string prevHash, std::vector<Transaction> txs) {
    index = idx;
    previousHash = prevHash;
    transactions = txs;
    timestamp = time(nullptr);
    nonce = 0;
    merkleRoot = computeMerkleRoot();
};

std::string Block::computeMerkleRoot() const {
    if (transactions.empty()) {
        return sha256("");
    }

    // Hash each transaction
    std::vector<std::string> hashes;
    for (const Transaction& tx : transactions) {
        hashes.push_back(sha256(tx.toString()));
    }

    // Repeatedly pair and hash until one remains
    while (hashes.size() > 1) {
        // If the size is odd, copy the last tx
        if (hashes.size() % 2 != 0) {
            hashes.push_back(hashes.back());
        }

        // Build the Merkle tree from the ground up
        std::vector<std::string> newLevel;
        for (size_t i = 0; i < hashes.size(); i += 2) {
            // Pair 1, 2 then 3, 4 then etc.
            newLevel.push_back(sha256(hashes[i] + hashes[i + 1]));
        }
        hashes = newLevel;
    }

    // Return Merkle Root
    return hashes[0];
}

std::string Block::calculateHash() const {
    std::string toHash =
        std::to_string(index) +
        std::to_string(timestamp) +
        std::to_string(nonce) +
        merkleRoot +
        previousHash;

    return sha256(toHash);
}

void Block::mineBlock(int diff) {
    std::cout << "\nMining block..." << std::endl;

    merkleRoot = computeMerkleRoot();

    std::string target = "";
    for (int i = 0; i < diff; i++) {
        target += "0";
    }

    hash = calculateHash();

    while (true) {
      if (hash.substr(0, diff) == target) {
            std::cout << "Block mined! Hash: " << hash << std::endl;
            break;
        }
        else {
            nonce++;
            if (nonce % 100000 == 0) {
                std::cout << "Current nonce: " << nonce << std::endl;
            }
            hash = calculateHash();
        }
    }

    return;
}

void Block::addTransaction(Transaction tx) {
    transactions.push_back(tx);
    merkleRoot = computeMerkleRoot();
}

std::string Block::toJSON() const {
    std::string json = "";

    // Opening brace
    json += "{";

    // Add index field
    json += "\"index\":" + std::to_string(index) + ",";

    // Add previousHash field
    json += "\"previousHash\":\"" + previousHash + "\",";

    // Add hash field
    json += "\"hash\":\"" + hash + "\",";

    // Add timestamp field
    json += "\"timestamp\":" + std::to_string(timestamp) + ",";

    // Add nonce field
    json += "\"nonce\":" + std::to_string(nonce) + ",";

    // Add transactions field
    json += "\"transactions\":[";
    for (size_t i = 0; i < transactions.size(); i++) {
        json += transactions[i].toJSON();
        if (i < transactions.size() - 1) {
            json += ",";
        }
    }
    json += "]";

    // Closing brace
    json += "}";

    return json;
}

Block Block::fromJSON(const std::string& json) { 
    // Extract index
    size_t pos = json.find("\"index\":") + 8;
    size_t end = json.find(",", pos);
    int idx = std::stoi(json.substr(pos, end - pos));
    
    // Extract previousHash
    pos = json.find("\"previousHash\":\"") + 16;
    end = json.find("\"", pos);
    std::string prevHash = json.substr(pos, end - pos);
    
    // Extract hash
    pos = json.find("\"hash\":\"") + 8;
    end = json.find("\"", pos);
    std::string hash = json.substr(pos, end - pos);
    
    // Extract timestamp
    pos = json.find("\"timestamp\":") + 12;
    end = json.find(",", pos);
    std::time_t timestamp = std::stol(json.substr(pos, end - pos));
    
    // Extract nonce
    pos = json.find("\"nonce\":") + 8;
    end = json.find(",", pos);
    int nonce = std::stoi(json.substr(pos, end - pos));
    
    // Extract transactions array
    std::vector<Transaction> transactions;
    pos = json.find("\"transactions\":[") + 16;  // Find start of array
    end = json.find("]", pos);  // Find end of array
    std::string transactionsStr = json.substr(pos, end - pos);  // Extract array content
    
    // Parse individual transactions
    std::vector<Transaction> parsedTransactions;
    size_t txPos = 0;
    while ((txPos = transactionsStr.find("{", txPos)) != std::string::npos) {
        size_t txStart = txPos;
        size_t txEnd = transactionsStr.find("}", txPos);
        
        std::string txJson = transactionsStr.substr(txStart, txEnd - txStart + 1);
        parsedTransactions.push_back(Transaction::fromJSON(txJson));
        
        txPos = txEnd + 1;
    }
    
    // Create and return the Block
    Block block(idx, prevHash, parsedTransactions);
    block.hash = hash;
    block.timestamp = timestamp;
    block.nonce = nonce;
    block.merkleRoot = block.computeMerkleRoot();
    
    return block;
}