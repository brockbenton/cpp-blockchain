# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
LDFLAGS = -lssl -lcrypto -pthread

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
NETWORK_DIR = $(SRC_DIR)/network
EXAMPLES_DIR = examples
BUILD_DIR = build
BIN_DIR = bin

# Source files
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.cpp)
NETWORK_SOURCES = $(wildcard $(NETWORK_DIR)/*.cpp)
MAIN_SOURCE = $(SRC_DIR)/main.cpp

# Object files
CORE_OBJECTS = $(CORE_SOURCES:$(CORE_DIR)/%.cpp=$(BUILD_DIR)/%.o)
NETWORK_OBJECTS = $(NETWORK_SOURCES:$(NETWORK_DIR)/%.cpp=$(BUILD_DIR)/%.o)
MAIN_OBJECT = $(BUILD_DIR)/main.o

# Executables
MAIN_EXEC = $(BIN_DIR)/blockchain
TEST_NETWORK_EXEC = $(BIN_DIR)/test_network

# Default target
all: directories $(MAIN_EXEC)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Build main executable
$(MAIN_EXEC): $(CORE_OBJECTS) $(NETWORK_OBJECTS) $(MAIN_OBJECT)
	$(CXX) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built main blockchain application"

# Build test network
test_network: directories $(CORE_OBJECTS) $(NETWORK_OBJECTS) $(BUILD_DIR)/test_network.o
	$(CXX) $^ -o $(TEST_NETWORK_EXEC) $(LDFLAGS)
	@echo "✓ Built test network application"

# Compile core object files
$(BUILD_DIR)/%.o: $(CORE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile network object files
$(BUILD_DIR)/%.o: $(NETWORK_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile main
$(BUILD_DIR)/main.o: $(MAIN_SOURCE)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test network
$(BUILD_DIR)/test_network.o: $(EXAMPLES_DIR)/test_network.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "✓ Cleaned build artifacts"

# Run the application
run: $(MAIN_EXEC)
	./$(MAIN_EXEC)

# Help
help:
	@echo "Available targets:"
	@echo "  all          - Build main blockchain application (default)"
	@echo "  test_network - Build network test application"
	@echo "  clean        - Remove build artifacts"
	@echo "  run          - Build and run main application"
	@echo "  help         - Show this help message"

.PHONY: all directories clean run help test_network
