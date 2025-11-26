#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>

class Transaction {
    public:
        std::string sender; // who is sending
        std::string receiver; // who is receiving
        double amount; // how much is being sent
        time_t timestamp; // when transaction was created

        // Constructor
        Transaction(std::string sender, std::string receiver, double amount);

        // Converts transaction to readable string
        std::string toString() const;

        // Creates unique hash of the transaction
        std::string calculateHash() const;

        // Converts transaction to JSON format
        std::string toJSON() const;

        // Parses transaction from JSON format
        static Transaction fromJSON(const std::string& json);
};

#endif