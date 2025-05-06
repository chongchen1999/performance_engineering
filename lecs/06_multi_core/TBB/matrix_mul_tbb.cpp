#include <tbb/tbb.h>

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#define MATRIX_SIZE 1024

// Matrix data structures
double matrixA[MATRIX_SIZE][MATRIX_SIZE];
double matrixB[MATRIX_SIZE][MATRIX_SIZE];
double matrixC_sequential[MATRIX_SIZE][MATRIX_SIZE];
double matrixC_parallel[MATRIX_SIZE][MATRIX_SIZE];

// Initialize matrix with random values
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixA[i][j] = static_cast<double>(rand()) / RAND_MAX;
            matrixB[i][j] = static_cast<double>(rand()) / RAND_MAX;
            matrixC_sequential[i][j] = 0.0;
            matrixC_parallel[i][j] = 0.0;
        }
    }
}

// Sequential matrix multiplication
void sequential_matrix_mul() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixC_sequential[i][j] = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matrixC_sequential[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }
}

// TBB parallel matrix multiplication functor
class ParallelMatrixMultiply {
   private:
    const int grain_size;

   public:
    ParallelMatrixMultiply(int grain = 1) : grain_size(grain) {}

    void operator()(const tbb::blocked_range<int>& range) const {
        for (int i = range.begin(); i < range.end(); ++i) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrixC_parallel[i][j] = 0.0;
                for (int k = 0; k < MATRIX_SIZE; k++) {
                    matrixC_parallel[i][j] += matrixA[i][k] * matrixB[k][j];
                }
            }
        }
    }
};

// Verify results
bool verify_results() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            if (std::fabs(matrixC_sequential[i][j] - matrixC_parallel[i][j]) >
                0.000001) {
                return false;  // Not equal
            }
        }
    }
    return true;  // Equal
}

int main(int argc, char* argv[]) {
    int num_threads = 8;  // Default number of threads

    // Check if number of threads is provided as command line argument
    if (argc > 1) {
        num_threads = std::atoi(argv[1]);
        if (num_threads <= 0) {
            std::cout << "Invalid number of threads. Using default (8)."
                      << std::endl;
            num_threads = 8;
        }
    }

    std::cout << "Matrix Size: " << MATRIX_SIZE << " x " << MATRIX_SIZE
              << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;

    // Set the seed for random number generation
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    initialize_matrices();

    // ====== Sequential multiplication ======
    std::cout << "\nPerforming sequential matrix multiplication..."
              << std::endl;
    auto seq_start = std::chrono::high_resolution_clock::now();
    sequential_matrix_mul();
    auto seq_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_elapsed = seq_end - seq_start;
    std::cout << "Sequential execution time: " << seq_elapsed.count()
              << " seconds" << std::endl;

    // ====== Parallel multiplication with TBB ======
    std::cout << "\nPerforming parallel matrix multiplication with TBB using "
              << num_threads << " threads..." << std::endl;

    // Set the global thread count for TBB
    tbb::global_control global_limit(
        tbb::global_control::max_allowed_parallelism, num_threads);

    auto par_start = std::chrono::high_resolution_clock::now();

    // Use TBB's parallel_for to divide the work
    tbb::parallel_for(tbb::blocked_range<int>(0, MATRIX_SIZE),
                      ParallelMatrixMultiply(), tbb::auto_partitioner());

    auto par_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_elapsed = par_end - par_start;
    std::cout << "Parallel execution time: " << par_elapsed.count()
              << " seconds" << std::endl;

    // ====== Result verification ======
    std::cout << "\nVerifying results..." << std::endl;
    if (verify_results()) {
        std::cout << "Results match! The parallel implementation is correct."
                  << std::endl;
    } else {
        std::cout
            << "Results do not match! There is an error in the implementation."
            << std::endl;
    }

    // Calculate speedup
    double speedup = seq_elapsed.count() / par_elapsed.count();
    std::cout << "\nSpeedup achieved: " << speedup << "x" << std::endl;
    std::cout << "Efficiency: " << (speedup / num_threads) * 100 << "%"
              << std::endl;

    return 0;
}