#include "Blockchain.h"
#include <iostream>
#include <fstream>
#include <sstream>

Blockchain::Blockchain(int diff, double reward, bool createGenesis) {
    difficulty = diff;
    miningReward = reward;

    std::vector<Transaction> genesisTx;
    genesisTx.push_back(Transaction("SYSTEM", "Alice", miningReward)); // Initial reward to Alice
    genesisTx.push_back(Transaction("SYSTEM", "Bob", miningReward));   // Initial reward to Bob
    genesisTx.push_back(Transaction("SYSTEM", "Charlie", miningReward)); // Initial reward to Charlie

    if (createGenesis) {
        Block genesisBlock(0, "0", genesisTx);
        genesisBlock.mineBlock(difficulty);
        chain.push_back(genesisBlock);
    }
}


void Blockchain::addBlock(std::vector<Transaction> tx) {
    std::vector<Transaction> validTransactions;
    for (const Transaction& transaction : tx) {
        if (validateTransaction(transaction)) {
            validTransactions.push_back(transaction);
        } else {
            std::cout << "Invalid transaction skipped:  From: " << transaction.sender 
                      << "  To: " << transaction.receiver 
                      << "  Amount: " << transaction.amount << std::endl;
        }
    }

    Transaction rewardTx("SYSTEM", "MINER", miningReward);
    validTransactions.insert(validTransactions.begin(), rewardTx);

    if (validTransactions.size() == 1) {  // Only reward, no actual transactions
        std::cout << "\nNo valid transactions to add (only mining reward)..." << std::endl;
    }

    const Block& lastBlock = chain.back();
    Block newBlock { (lastBlock.index + 1), lastBlock.hash, validTransactions };
    newBlock.mineBlock(difficulty);
    chain.push_back(newBlock);
}

void Blockchain::printChain() {
    for (const Block& block : chain) {
        std::cout << "\n==================== Block " << block.index << " ====================" << std::endl;
        std::cout << "Block Index: " << block.index << std::endl;
        std::cout << "Transactions: " << std::endl;
        for (const Transaction& tx : block.transactions) {
            std::cout << "  From: " << tx.sender << "  To: " << tx.receiver << "  Amount: " << tx.amount << "  Timestamp: " << tx.timestamp << std::endl;
        }
        std::cout << "Previous Hash: " << block.previousHash << std::endl;
        std::cout << "Hash: " << block.hash << std::endl;
        std::cout << "Timestamp: " << block.timestamp << std::endl;
        std::cout << "Nonce: " << block.nonce << std::endl;
        std::cout << "=================================================" << std::endl;
    }
}

Block& Blockchain::getBlock(int idx) {
    return chain[idx];
}

bool Blockchain::isChainValid() {
    for (size_t i = 1; i < chain.size(); i++) {
        Block block = chain[i];
        Block prevBlock = chain[i-1];

        std::string originalHash = block.hash;
        std::string freshHash = block.calculateHash();
        if (originalHash != freshHash) {
            std::cout << "\nBlock " << i << "'s data has been tampered with!" << std::endl;
            return false;
        }

        std::string curPrevHash = block.previousHash;
        std::string prevHash = prevBlock.hash;
        if (curPrevHash != prevHash) {
            std::cout << "Block " << i << " has invalid previous hash link!" << std::endl;
            return false;
        }

        std::string target = "";
        for (int j = 0; j < difficulty; j++) {
            target += "0";
        }
        if (originalHash.substr(0, difficulty) != target) {
             std::cout << "Block " << i << " doesn't meet difficulty requirements!" << std::endl;
            return false;
        }
    }
    return true;
}

bool Blockchain::isValidChain(const std::vector<Block>& testChain) const {
    if (testChain.empty()) {
        return false;
    }
    
    for (size_t i = 1; i < testChain.size(); i++) {
        Block block = testChain[i];
        Block prevBlock = testChain[i-1];

        std::string originalHash = block.hash;
        std::string freshHash = block.calculateHash();
        if (originalHash != freshHash) {
            return false;
        }

        std::string curPrevHash = block.previousHash;
        std::string prevHash = prevBlock.hash;
        if (curPrevHash != prevHash) {
            return false;
        }

        std::string target = "";
        for (int j = 0; j < difficulty; j++) {
            target += "0";
        }
        if (originalHash.substr(0, difficulty) != target) {
            return false;
        }
    }
    return true;
}

double Blockchain::getBalance(std::string address) {
    double balance = 0.0;

    for (const Block& block : chain) {
        for (const Transaction& tx : block.transactions) {
            if (tx.sender == address) {
                balance -= tx.amount;
            }
            if (tx.receiver == address) {
                balance += tx.amount;
            }
        }
    }

    return balance;
}

bool Blockchain::validateTransaction(Transaction tx) {
    if(tx.sender == "SYSTEM") {
        return true; // System transactions are always valid
    }

    double senderBalance = getBalance(tx.sender);

    if (senderBalance >= tx.amount) {
        return true;
    } 
    else {
        std::cout << "Transaction from " << tx.sender << " to " << tx.receiver 
        << " for amount " << tx.amount 
        << " is invalid due to insufficient balance." << std::endl;
        return false;
    }

}


bool Blockchain::saveToFile(const std::string& filename) const {
    // Open file for writing
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Save each block in chains data
    file << "[\n";

    for (size_t i = 0; i < chain.size(); i++) {
        file << " " << chain[i].toJSON();

        if (i < chain.size() - 1) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }

    file << "]";
    file.close();

    return true;
}

bool Blockchain::loadFromFile(const std::string& filename) {
    // Open file for reading
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Read entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Find the array boundaries - use rfind for last ] to handle nested arrays in transactions
    size_t arrayStart = content.find("[");
    size_t arrayEnd = content.rfind("]");

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        return false; // Invalid JSON
    }

    // Extract content between [ and ]
    std::string blocksContent = content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    // Parse individual block JSON objects
    std::vector<Block> loadedBlocks;

    size_t blockStart = 0;
    int braceCount = 0;

    // Iterate through content character by character
    for (size_t i = 0; i < blocksContent.length(); i++) {
        char c = blocksContent[i];

        if (c == '{') {
            if (braceCount == 0) {
                // Mark the start of a block
                blockStart = i;
            }
            braceCount++;
        }
        else if (c == '}') {
            braceCount--;

            if (braceCount == 0) {
                // We've found a complete block
                std::string blockJson = blocksContent.substr(blockStart, i - blockStart + 1);

                // Convert JSON to Block and add to loaded blocks
                Block block = Block::fromJSON(blockJson);
                loadedBlocks.push_back(block);
            }
        }
    }

    // Replace the current chain with loaded blocks
    chain.clear();
    chain = loadedBlocks;

    return true;
}

std::string Blockchain::toJSON() const {
    std::string json = "[";
    
    for (size_t i = 0; i < chain.size(); i++) {
        json += chain[i].toJSON();
        if (i < chain.size() - 1) {
            json += ",";
        }
    }
    
    json += "]";
    return json;
}

void Blockchain::replaceChain(const std::vector<Block>& newChain) {
    chain.clear();
    chain = newChain;
}

void Blockchain::addExistingBlock(const Block& block) {
    chain.push_back(block);
}
