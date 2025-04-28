#include <gtest/gtest.h>

#include <chrono>
#include <iostream>

#include "matrix_multiplication.h"

// For CPU feature detection
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

// Helper functions for test setup
Matrix createRandomMatrix(int rows, int cols) {
    Matrix mat(rows, cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat.at(i, j) = static_cast<double>(rand()) / RAND_MAX;
        }
    }

    return mat;
}

// Check if two matrices are approximately equal
bool matricesEqual(const Matrix& A, const Matrix& B, double tolerance = 1e-10) {
    if (A.rows != B.rows || A.cols != B.cols) {
        return false;
    }

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < A.cols; j++) {
            if (std::abs(A.at(i, j) - B.at(i, j)) > tolerance) {
                std::cout << "Difference at [" << i << "][" << j
                          << "]: " << A.at(i, j) << " vs " << B.at(i, j)
                          << std::endl;
                return false;
            }
        }
    }

    return true;
}

// Benchmark helper
template <typename Func>
double benchmark(Func func, int repeat = 3) {
    using namespace std::chrono;

    double total_time = 0.0;
    for (int i = 0; i < repeat; i++) {
        auto start = high_resolution_clock::now();
        func();
        auto end = high_resolution_clock::now();
        total_time += duration_cast<milliseconds>(end - start).count();
    }

    return total_time / repeat;  // Average time in milliseconds
}

// Test correctness
TEST(MatrixMultiplicationTest, CorrectnessTest) {
    // Small matrices for verification
    Matrix A = createRandomMatrix(10, 10);
    Matrix B = createRandomMatrix(10, 10);

    // Compute reference result
    Matrix naive_result = naive_matrix_multiply(A, B);

    // Test all implementations
    Matrix loop_result = loop_interchange_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, loop_result));

    Matrix parallel_result = parallel_loop_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, parallel_result));

    Matrix tiled_result = tiled_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, tiled_result));

    Matrix dc_result = divide_conquer_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, dc_result));

    Matrix avx_result = avx2_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, avx_result));

    Matrix opt_result = optimized_matrix_multiply(A, B);
    EXPECT_TRUE(matricesEqual(naive_result, opt_result));
}

// Test invalid dimensions
TEST(MatrixMultiplicationTest, IncompatibleDimensions) {
    Matrix A = createRandomMatrix(10, 20);
    Matrix B = createRandomMatrix(30, 10);

    EXPECT_THROW(naive_matrix_multiply(A, B), std::invalid_argument);
    EXPECT_THROW(optimized_matrix_multiply(A, B), std::invalid_argument);
}

// Performance test
TEST(MatrixMultiplicationTest, PerformanceTest) {
    // Larger matrices for benchmarking
    constexpr int size = 1024;
    Matrix A = createRandomMatrix(size, size);
    Matrix B = createRandomMatrix(size, size);

    // Warm up
    Matrix C = naive_matrix_multiply(A, B);

    // Benchmark all implementations
    double naive_time = benchmark([&]() { naive_matrix_multiply(A, B); });
    double loop_time =
        benchmark([&]() { loop_interchange_matrix_multiply(A, B); });
    double parallel_time =
        benchmark([&]() { parallel_loop_matrix_multiply(A, B); });
    double tiled_time = benchmark([&]() { tiled_matrix_multiply(A, B); });
    double dc_time = benchmark([&]() { divide_conquer_matrix_multiply(A, B); });
    double avx_time = benchmark([&]() { avx2_matrix_multiply(A, B); });
    double opt_time = benchmark([&]() { optimized_matrix_multiply(A, B); });

    std::cout << "Performance Results (ms):" << std::endl;
    std::cout << "Naive: " << naive_time << std::endl;
    std::cout << "Loop Interchange: " << loop_time << std::endl;
    std::cout << "Parallel Loop: " << parallel_time << std::endl;
    std::cout << "Tiled: " << tiled_time << std::endl;
    std::cout << "Divide & Conquer: " << dc_time << std::endl;
    std::cout << "AVX2: " << avx_time << std::endl;
    std::cout << "Optimized: " << opt_time << std::endl;

    // The optimized implementation should be faster than naive
    EXPECT_LT(opt_time, naive_time);
}

int main(int argc, char** argv) {
// Check if AVX2 is supported on this CPU
#ifdef __AVX2__
    std::cout << "AVX2 is supported by the compiler." << std::endl;

// Runtime check for AVX2 support
#if defined(_MSC_VER)
    // Windows/MSVC
    int cpuInfo[4];
    __cpuid(cpuInfo, 7);
    bool avx2Supported = (cpuInfo[1] & (1 << 5)) != 0;
#else
    // GCC/Clang
    bool avx2Supported = __builtin_cpu_supports("avx2");
#endif

    if (!avx2Supported) {
        std::cout << "WARNING: Your CPU does not support AVX2 instructions."
                  << std::endl;
        std::cout << "The AVX2 and optimized tests will be skipped."
                  << std::endl;
    } else {
        std::cout << "AVX2 is supported by the CPU." << std::endl;
    }
#else
    std::cout << "WARNING: This binary was not compiled with AVX2 support."
              << std::endl;
    std::cout << "The AVX2 and optimized tests will fail." << std::endl;
#endif

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}