CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -pthread
BUILD_DIR = ./build

all: prepare $(BUILD_DIR)/parallel_quicksort

prepare:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/parallel_quicksort: parallel_quicksort.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all prepare clean