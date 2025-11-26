#include "Block.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <openssl/sha.h> // Bitcoin uses SHA-256

Block::Block(int idx, std::string prevHash, std::vector<Transaction> txs) {
    index = idx;
    previousHash = prevHash;
    transactions = txs;
    timestamp = time(nullptr);
    nonce = 0;
    // hash = calculateHash();
};

std::string Block::calculateHash() const {
    // Concatenate
    std::string toHash = 
        std::to_string(index) + 
        std::to_string(timestamp) + 
        std::to_string(nonce) +
        getTransactionsAsString() +
        previousHash;
    
    // SHA256 Function
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, toHash.c_str(), toHash.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

void Block::mineBlock(int diff) {
    std::cout << "\nMining block..." << std::endl;

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
}

std::string Block::getTransactionsAsString() const {
    std::string txString = "";
    for (const Transaction& tx : transactions) {
        txString += tx.toString();
    }
    return txString;
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
    
    return block;
}