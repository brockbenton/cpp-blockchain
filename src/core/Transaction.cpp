#include "Transaction.h"
#include <openssl/sha.h>
#include <string>
#include <sstream>
#include <iomanip>

Transaction::Transaction(std::string sdr, std::string rcv, double amt) {
    sender = sdr;
    receiver = rcv;
    amount = amt;
    timestamp = time(nullptr);
}

std::string Transaction::toString() const {
    std::string stringAmount = std::to_string(amount);
    return stringAmount;
}

std::string Transaction::calculateHash() const {
    // Concatenate
    std::string toHash = 
        std::to_string(amount) + 
        std::to_string(timestamp) + 
        sender + 
        receiver;
    
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

std::string Transaction::toJSON() const {
    std::string json = "";

    // Opening brace
    json += "{";

    // Add sender field
    json += "\"sender\":\"" + sender + "\",";

    // Add receiver field
    json += "\"receiver\":\"" + receiver + "\",";

    // Add amount field
    json += "\"amount\":" + std::to_string(amount) + ",";

    // Add timestamp field
    json += "\"timestamp\":" + std::to_string(timestamp);

    // Closing brace
    json += "}";

    return json;
}

Transaction Transaction::fromJSON(const std::string& json) {
    // Extract sender
    size_t pos = json.find("\"sender\":\"") + 10;
    size_t end = json.find("\"", pos);
    std::string sender = json.substr(pos, end - pos);
    
    // Extract receiver
    pos = json.find("\"receiver\":\"") + 12;
    end = json.find("\"", pos);
    std::string receiver = json.substr(pos, end - pos);
    
    // Extract amount
    pos = json.find("\"amount\":") + 9;
    end = json.find_first_of(",}", pos);
    double amount = std::stod(json.substr(pos, end - pos));
    
    // Extract timestamp
    pos = json.find("\"timestamp\":") + 12;
    end = json.find_first_of(",}", pos);
    time_t timestamp = std::stol(json.substr(pos, end - pos));
    
    Transaction tx(sender, receiver, amount);
    tx.timestamp = timestamp;  // Set timestamp after construction
    return tx;
}