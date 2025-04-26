/**
 * Optimized Matrix Multiplication Implementation
 *
 * This implementation includes:
 * - Loop interchange
 * - Tiling
 * - Parallel execution (OpenMP)
 * - Compiler optimizations
 * - Compiler vectorization hints
 * - AVX intrinsics
 * - Parallel divide-and-conquer approach
 *
 * Comparison with Intel MKL for 4096x4096 matrices
 */

#include <immintrin.h>  // For AVX intrinsics
#include <mkl.h>        // For Intel MKL comparison
#include <omp.h>

#include <chrono>
#include <iostream>
#include <vector>

// Constants for optimization
constexpr int TILE_SIZE = 64;     // Cache-friendly tile size
constexpr int MIN_DC_SIZE = 512;  // Minimum size for divide-and-conquer

// Simple/baseline matrix multiplication (for comparison)
void matrix_multiply_baseline(const float *A, const float *B, float *C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            float sum = 0.0f;
            for (int k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

// Loop interchange optimization
void matrix_multiply_loop_interchange(const float *A, const float *B, float *C,
                                      int N) {
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            for (int j = 0; j < N; j++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

// Cache blocking/tiling optimization
void matrix_multiply_tiled(const float *A, const float *B, float *C, int N) {
// Initialize C to zeros
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0f;
        }
    }

// Tiled multiplication
#pragma omp parallel for collapse(3)
    for (int i = 0; i < N; i += TILE_SIZE) {
        for (int j = 0; j < N; j += TILE_SIZE) {
            for (int k = 0; k < N; k += TILE_SIZE) {
                // Process a tile
                for (int ii = i; ii < std::min(i + TILE_SIZE, N); ii++) {
                    for (int kk = k; kk < std::min(k + TILE_SIZE, N); kk++) {
                        for (int jj = j; jj < std::min(j + TILE_SIZE, N);
                             jj++) {
                            C[ii * N + jj] += A[ii * N + kk] * B[kk * N + jj];
                        }
                    }
                }
            }
        }
    }
}

// AVX-optimized matrix multiplication
void matrix_multiply_avx(const float *A, const float *B, float *C, int N) {
// Initialize C to zeros
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0f;
        }
    }

    // Make sure N is a multiple of 8 for AVX
    const int NR = N - (N % 8);

#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            // Broadcast A[i,k] to all lanes of a vector
            __m256 a = _mm256_set1_ps(A[i * N + k]);

            // Process 8 elements of B and C at a time using AVX
            for (int j = 0; j < NR; j += 8) {
                // Load 8 elements from C
                __m256 c = _mm256_loadu_ps(&C[i * N + j]);

                // Load 8 elements from B
                __m256 b = _mm256_loadu_ps(&B[k * N + j]);

                // Compute a * b + c and store back to C
                c = _mm256_fmadd_ps(a, b, c);
                _mm256_storeu_ps(&C[i * N + j], c);
            }

            // Handle remaining elements
            for (int j = NR; j < N; j++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

// Recursive divide-and-conquer matrix multiplication
void matrix_multiply_dc(const float *A, const float *B, float *C, int N,
                        int rowA, int colA, int rowB, int colB, int rowC,
                        int colC) {
    if (N <= MIN_DC_SIZE) {
        // Base case: use AVX on smaller matrices
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                __m256 a = _mm256_set1_ps(A[(rowA + i) * N + (colA + k)]);

                int j = 0;
                for (; j <= N - 8; j += 8) {
                    __m256 c = _mm256_loadu_ps(&C[(rowC + i) * N + (colC + j)]);
                    __m256 b = _mm256_loadu_ps(&B[(rowB + k) * N + (colB + j)]);
                    c = _mm256_fmadd_ps(a, b, c);
                    _mm256_storeu_ps(&C[(rowC + i) * N + (colC + j)], c);
                }

                // Handle remaining elements
                for (; j < N; j++) {
                    C[(rowC + i) * N + (colC + j)] +=
                        A[(rowA + i) * N + (colA + k)] *
                        B[(rowB + k) * N + (colB + j)];
                }
            }
        }
        return;
    }

    // Divide the matrices into quadrants
    int newN = N / 2;

// Use OpenMP tasks for parallel divide-and-conquer
#pragma omp parallel sections if (N >= 1024)
    {
#pragma omp section
        {
            // C11 = A11 * B11 + A12 * B21
            matrix_multiply_dc(A, B, C, newN, rowA, colA, rowB, colB, rowC,
                               colC);
            matrix_multiply_dc(A, B, C, newN, rowA, colA + newN, rowB + newN,
                               colB, rowC, colC);
        }

#pragma omp section
        {
            // C12 = A11 * B12 + A12 * B22
            matrix_multiply_dc(A, B, C, newN, rowA, colA, rowB, colB + newN,
                               rowC, colC + newN);
            matrix_multiply_dc(A, B, C, newN, rowA, colA + newN, rowB + newN,
                               colB + newN, rowC, colC + newN);
        }

#pragma omp section
        {
            // C21 = A21 * B11 + A22 * B21
            matrix_multiply_dc(A, B, C, newN, rowA + newN, colA, rowB, colB,
                               rowC + newN, colC);
            matrix_multiply_dc(A, B, C, newN, rowA + newN, colA + newN,
                               rowB + newN, colB, rowC + newN, colC);
        }

#pragma omp section
        {
            // C22 = A21 * B12 + A22 * B22
            matrix_multiply_dc(A, B, C, newN, rowA + newN, colA, rowB,
                               colB + newN, rowC + newN, colC + newN);
            matrix_multiply_dc(A, B, C, newN, rowA + newN, colA + newN,
                               rowB + newN, colB + newN, rowC + newN,
                               colC + newN);
        }
    }
}

// Entry point function for divide-and-conquer
void matrix_multiply_divide_conquer(const float *A, const float *B, float *C,
                                    int N) {
// Initialize C to zeros
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0f;
        }
    }

// Make sure N is a power of 2 (for simplicity in the divide-and-conquer)
// In a production environment, you'd handle arbitrary sizes

// Start the recursive multiplication
#pragma omp parallel
    {
#pragma omp single
        { matrix_multiply_dc(A, B, C, N, 0, 0, 0, 0, 0, 0); }
    }
}

// Final combined optimized version
void matrix_multiply_optimized(const float *A, const float *B, float *C,
                               int N) {
// Initialize C to zeros
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i * N + j] = 0.0f;
        }
    }

// Use tiling + loop interchange + AVX + OpenMP
#pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i += TILE_SIZE) {
        for (int j = 0; j < N; j += TILE_SIZE) {
            // Process each tile
            for (int k = 0; k < N; k += TILE_SIZE) {
                for (int ii = i; ii < std::min(i + TILE_SIZE, N); ii++) {
                    for (int kk = k; kk < std::min(k + TILE_SIZE, N); kk++) {
                        // Broadcast A[ii,kk] to all lanes of a vector
                        __m256 a = _mm256_set1_ps(A[ii * N + kk]);

                        // Process 8 elements at a time using AVX
                        int jj = j;
                        for (; jj < std::min(j + TILE_SIZE, N) - 7; jj += 8) {
                            __m256 c = _mm256_loadu_ps(&C[ii * N + jj]);
                            __m256 b = _mm256_loadu_ps(&B[kk * N + jj]);

                            // Use FMA instruction for better performance
                            c = _mm256_fmadd_ps(a, b, c);
                            _mm256_storeu_ps(&C[ii * N + jj], c);
                        }

                        // Handle remaining elements
                        for (; jj < std::min(j + TILE_SIZE, N); jj++) {
                            C[ii * N + jj] += A[ii * N + kk] * B[kk * N + jj];
                        }
                    }
                }
            }
        }
    }
}

// Function to compare with Intel MKL
void matrix_multiply_mkl(const float *A, const float *B, float *C, int N) {
    float alpha = 1.0f;
    float beta = 0.0f;

    // Use MKL's SGEMM (Single precision GEneral Matrix Multiply)
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, alpha, A, N,
                B, N, beta, C, N);
}

int main() {
    // Test with 4096x4096 matrices as requested
    const int N = 4096;

    std::cout << "Allocating memory for " << N << "x" << N << " matrices..."
              << std::endl;

    // Allocate memory with proper alignment for AVX
    float *A = (float *)aligned_alloc(32, N * N * sizeof(float));
    float *B = (float *)aligned_alloc(32, N * N * sizeof(float));
    float *C1 = (float *)aligned_alloc(32, N * N * sizeof(float));
    float *C2 = (float *)aligned_alloc(32, N * N * sizeof(float));

    if (!A || !B || !C1 || !C2) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }

    // Initialize matrices with random data
    srand(42);  // For reproducibility
#pragma omp parallel for
    for (int i = 0; i < N * N; i++) {
        A[i] = static_cast<float>(rand()) / RAND_MAX;
        B[i] = static_cast<float>(rand()) / RAND_MAX;
    }

    // Set number of threads
    int num_threads = omp_get_max_threads();
    std::cout << "Running with " << num_threads << " threads" << std::endl;

    // --------------------- Optimized Version ---------------------
    auto start = std::chrono::high_resolution_clock::now();

    matrix_multiply_optimized(A, B, C1, N);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Optimized matrix multiplication time: " << elapsed.count()
              << " seconds" << std::endl;

    // --------------------- MKL Version ---------------------
    start = std::chrono::high_resolution_clock::now();

    matrix_multiply_mkl(A, B, C2, N);

    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Intel MKL matrix multiplication time: " << elapsed.count()
              << " seconds" << std::endl;

    // Validate results
    float max_diff = 0.0f;
    for (int i = 0; i < N * N; i++) {
        max_diff = std::max(max_diff, std::abs(C1[i] - C2[i]));
    }

    std::cout << "Maximum difference between optimized and MKL: " << max_diff
              << std::endl;

    // Speed comparison
    std::cout << "\nPerformance Analysis:" << std::endl;

    // Calculate GFLOPS
    double gflops =
        (2.0 * N * N * N) / 1e9;  // 2*N^3 operations for matrix multiplication
    std::cout << "Optimized Implementation: " << (gflops / elapsed.count())
              << " GFLOPS" << std::endl;

    // Clean up
    free(A);
    free(B);
    free(C1);
    free(C2);

    return 0;
}