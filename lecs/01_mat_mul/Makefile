# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -O3 -march=native -mavx2 -fopenmp -Wall -Wextra -fpermissive

# Include directories
INCLUDES = -I. -I/usr/local/include

# Library directories
LDFLAGS = -L/usr/local/lib

# Libraries
LIBS = -lgtest -lgtest_main -pthread -fopenmp

# Source files
SOURCES = matrix_mult_test.cpp

# Output executable
EXECUTABLE = matrix_test

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES) matrix_multiplication.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SOURCES) $(LDFLAGS) $(LIBS)

test: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all test clean