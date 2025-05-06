CXX = g++
CXXFLAGS = -Wall -Wextra -O3 -std=c++17
LDFLAGS = -ltbb

# Output directory
BUILD_DIR = ./build

# Source files
SRC = matrix_mul_tbb.cpp

# Output executable
TARGET = $(BUILD_DIR)/matrix_mul_tbb

# Default target
all: setup $(TARGET)

# Setup build directory
setup:
	mkdir -p $(BUILD_DIR)

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the program with default parameters
run: all
	$(TARGET)

# Run with specific number of threads
# Usage: make run_threads THREADS=8
run_threads: all
	$(TARGET) $(THREADS)

.PHONY: all setup clean run run_threads