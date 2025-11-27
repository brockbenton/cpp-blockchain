# Distributed Blockchain Network

A fully functional proof-of-work blockchain implementation in C++ featuring peer-to-peer networking, transaction validation, and distributed consensus.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

## 🌟 Features

- **Proof-of-Work Mining**: SHA-256 based cryptographic mining with adjustable difficulty
- **Transaction System**: Full transaction support with sender/receiver validation and balance tracking
- **Peer-to-Peer Network**: TCP socket-based distributed architecture with automatic chain synchronization
- **Multi-Threading**: Concurrent peer handling using C++ threads and mutex locks
- **Chain Validation**: Cryptographic integrity verification and tamper detection
- **Persistence**: JSON-based blockchain serialization for saving/loading chain state
- **Mining Rewards**: Automatic coinbase transactions for block miners

## 🏗️ Architecture
```
┌─────────────────────────────────────────────────────────┐
│                     Blockchain Node                     │
├─────────────────────────────────────────────────────────┤
│  ┌────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │   Block    │  │ Transaction  │  │   Blockchain    │  │
│  │            │  │              │  │                 │  │
│  │ - index    │  │ - sender     │  │ - chain[]       │  │
│  │ - hash     │  │ - receiver   │  │ - difficulty    │  │
│  │ - prevHash │  │ - amount     │  │ - addBlock()    │  │
│  │ - nonce    │  │ - timestamp  │  │ - isValid()     │  │
│  │ - txs[]    │  └──────────────┘  └─────────────────┘  │
│  └────────────┘                                         │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │              Network Layer (Node)                 │  │
│  │  ┌────────────────┐  ┌────────────────┐           │  │
│  │  │ Listener Thread│  │  Peer Handlers │           │  │
│  │  │                │  │                │           │  │
│  │  │ - accept()     │  │ - recv()       │           │  │
│  │  │ - spawn        │  │ - parseMsg()   │           │  │
│  │  │   handlers     │  │ - broadcast()  │           │  │
│  │  └────────────────┘  └────────────────┘           │  │
│  └───────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Core Components

1. **Block**: Individual block in the chain containing transactions, hash, and proof-of-work
2. **Transaction**: Transfer of value between addresses with timestamp
3. **Blockchain**: Chain of blocks with validation, mining, and consensus logic
4. **Node**: Network-enabled blockchain node with P2P communication

## 🚀 Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 5+)
- OpenSSL development libraries
- POSIX threads support (pthreads)

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install build-essential libssl-dev
```

**macOS:**
```bash
brew install openssl
```

### Building
```bash
# Clone the repository
git clone https://github.com/brockbenton/cpp-blockchain.git
cd cpp-blockchain

# Build using Make
make

# Or build manually
g++ -o bin/blockchain src/main.cpp src/core/*.cpp src/network/*.cpp -lssl -lcrypto -pthread -std=c++17
```

### Running

**Single Node:**
```bash
./bin/blockchain
# Follow interactive prompts
```

**Multi-Node Network:**

Terminal 1:
```bash
./bin/blockchain
# Port: 8080
# Difficulty: 2
# Reward: 50
```

Terminal 2:
```bash
./bin/blockchain
# Port: 8081
# Difficulty: 2
# Reward: 50
# Then: Option 6 → Connect to 127.0.0.1:8080
```

## 📖 Usage Examples

### Creating Transactions
```
1. Create transaction
From: Alice
To: Bob
Amount: 25
```

### Mining Blocks
```
2. Mine pending transactions
⛏ Mining block with 1 transactions...
Block mined! Hash: 00a3f5d8c9e2b4...
✓ Block mined and broadcast to network!
```

### Checking Balances
```
4. Check balance
Enter address: Alice
💰 Balance of Alice: 75.0
```

### Network Operations
```
6. Connect to peer
Peer IP: 127.0.0.1
Peer port: 8080
✓ Connected to peer!
```

## 🔬 Technical Deep Dive

### Proof-of-Work Algorithm

The blockchain uses SHA-256 based proof-of-work. Miners must find a nonce that produces a hash meeting the difficulty requirement:
```cpp
std::string Block::mineBlock(int difficulty) {
    std::string target(difficulty, '0');
    while (hash.substr(0, difficulty) != target) {
        nonce++;
        hash = calculateHash();
    }
}
```

### Network Protocol

Nodes communicate using JSON messages over TCP:
```json
{"type":"NEW_BLOCK","data":{block}}
{"type":"GET_CHAIN"}
{"type":"CHAIN","data":[blocks]}
{"type":"GET_LENGTH"}
{"type":"LENGTH","value":5}
```

### Chain Synchronization

On connection, nodes:
1. Exchange chain lengths
2. Request full chain if peer is longer
3. Validate received chain
4. Adopt if longer and valid

### Thread Safety

Critical sections protected with mutexes:
- `chainMutex`: Protects blockchain modifications
- `peersMutex`: Protects peer socket list

## 🧪 Testing

Build and run the network test:
```bash
make test_network
./bin/test_network
```

This creates 3 interconnected nodes and demonstrates:
- Peer discovery and connection
- Block mining and propagation
- Chain synchronization
- Distributed consensus

## 📊 Performance

**Mining Performance** (difficulty 4, single thread):
- Average time: 10-30 seconds per block
- Hash rate: ~50,000 hashes/second

**Network Latency**:
- Local peers: <100ms for block propagation
- Chain sync: <1s for 10-block chain

## 🛠️ Project Structure
```
cpp-blockchain/
├── src/
│   ├── core/              # Core blockchain logic
│   │   ├── Block.*        # Block implementation
│   │   ├── Blockchain.*   # Blockchain & validation
│   │   └── Transaction.*  # Transaction handling
│   ├── network/           # P2P networking
│   │   └── Node.*         # Node & protocol
│   └── main.cpp           # CLI application
├── examples/              # Example programs
├── Makefile               # Build system
└── README.md
```

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- UTXO transaction model
- Merkle trees for efficient verification
- Enhanced fork resolution
- GUI frontend
- Smart contracts

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Built as a learning project to understand:
- Blockchain fundamentals
- Distributed systems
- Network programming
- Cryptographic applications

## 📧 Contact

Brock - [https://brockbenton.github.io]

Project Link: [https://github.com/brockbenton/cpp-blockchain]

---

⭐ If you found this project helpful, consider giving it a star!

